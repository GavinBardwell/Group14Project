#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <algorithm>

#include <caliper/cali.h>
#include <caliper/cali-manager.h>
#include <adiak.hpp>

#define MASTER 0               /* taskid of first task */
#define MAX_DIGIT 2147483647         /* Number of possible values for each digit (base) */

void generateData(int task_id, int num_procs, int array_size, int length_for_local_array, std::vector<int>& local_data, int presorted) {
    int base_length = array_size / (num_procs - 1);
    int remainder = array_size % (num_procs - 1);
    int start_index = (task_id - 1) * base_length + ((task_id - 1) < remainder ? (task_id - 1) : remainder);

    if (presorted == 1) { // Sorted
        for (int i = 0; i < length_for_local_array; i++) {
            local_data.push_back(start_index + i); // Generate sorted values
        }
    } else if (presorted == 2) { // Reverse sorted
        for (int i = 0; i < length_for_local_array; i++) {
            local_data.push_back(array_size - (start_index + i) - 1); // Generate reverse sorted values
        }
    } else if (presorted == 3) { // 1% perturbed
        for (int i = 0; i < length_for_local_array; i++) {
            local_data.push_back(start_index + i); // Start with sorted values
        }
        // Perturb 1% of the elements
        int perturb_count = length_for_local_array / 100;
        for (int i = 0; i < perturb_count; i++) {
            int idx1 = rand() % length_for_local_array;
            int idx2 = rand() % length_for_local_array;
            std::swap(local_data[idx1], local_data[idx2]);
        }
    } else { // Random
        for (int i = 0; i < length_for_local_array; i++) {
            local_data.push_back(rand() % MAX_DIGIT); // Generate random values
        }
    }
}

void writeDataToFile(const char *file_name, const std::vector<int>& data) {
    FILE *file = fopen(file_name, "w");
    if (file == NULL) {
        printf("Error: Could not open file %s for writing.\n", file_name);
        return;
    }
    
    for (size_t i = 0; i < data.size(); i++) {
        if (i < data.size() - 1) {
            fprintf(file, "%d,", data[i]);
        } else {
            fprintf(file, "%d\n", data[i]);
        }
    }
    
    fclose(file);
}
void parallel_radix_sort(std::vector<int>& local_data, int max_bits, MPI_Comm comm) {
    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);
    int local_size = local_data.size();

    for (int bit = 0; bit < max_bits; ++bit) {
        std::vector<int> zero_bucket, one_bucket;

        // Split local data into zero and one buckets based on the current bit
        for (int num : local_data) {
            if (num & (1 << bit)) {
                one_bucket.push_back(num);
            } else {
                zero_bucket.push_back(num);
            }
        }

        // Gather sizes of zero and one buckets from all processes
        int zero_bucket_size = zero_bucket.size();
        int one_bucket_size = one_bucket.size();
        std::vector<int> all_zero_sizes(size), all_one_sizes(size);
        MPI_Allgather(&zero_bucket_size, 1, MPI_INT, all_zero_sizes.data(), 1, MPI_INT, comm);
        MPI_Allgather(&one_bucket_size, 1, MPI_INT, all_one_sizes.data(), 1, MPI_INT, comm);

        // Calculate displacements and total sizes for zero and one buckets
        int total_zero_size = 0, total_one_size = 0;
        std::vector<int> zero_displs(size), one_displs(size);
        for (int i = 0; i < size; ++i) {
            zero_displs[i] = total_zero_size;
            one_displs[i] = total_one_size;
            total_zero_size += all_zero_sizes[i];
            total_one_size += all_one_sizes[i];
        }

        // Gather all zero and one bucket elements
        std::vector<int> all_zeros(total_zero_size), all_ones(total_one_size);
        MPI_Allgatherv(zero_bucket.data(), zero_bucket_size, MPI_INT, all_zeros.data(), all_zero_sizes.data(), zero_displs.data(), MPI_INT, comm);
        MPI_Allgatherv(one_bucket.data(), one_bucket_size, MPI_INT, all_ones.data(), all_one_sizes.data(), one_displs.data(), MPI_INT, comm);

        // Redistribute data to maintain equal size per process
        local_data.clear();
        local_data.insert(local_data.end(), all_zeros.begin(), all_zeros.end());
        local_data.insert(local_data.end(), all_ones.begin(), all_ones.end());

        // Correctly distribute data across processes
        int total_data_size = local_data.size();
        int chunk_size = total_data_size / size;
        int remainder = total_data_size % size;
        int start_index = rank * chunk_size + std::min(rank, remainder);
        int end_index = start_index + chunk_size + (rank < remainder ? 1 : 0);
        local_data = std::vector<int>(local_data.begin() + start_index, local_data.begin() + end_index);

        // Synchronize processes before moving to the next bit
        MPI_Barrier(comm);
    }
}

void checkSorted(const std::vector<int>& data, int original_size) {
    bool is_sorted = true;
    for (size_t i = 1; i < data.size(); ++i) {
        if (data[i] < data[i - 1]) {
            is_sorted = false;
            break;
        }
    }

    if (is_sorted && data.size() == original_size) {
        std::cout << "Array is sorted correctly and has the correct length." << std::endl;
    } else if (!is_sorted && data.size() == original_size) {
        std::cout << "Array is NOT sorted correctly." << std::endl;
    } else if(data.size() != original_size) {
        std::cout << "Array is sorted but does NOT have the correct length." << std::endl;
    }
    else {
        std::cout <<"Array is wrong size and not sorted" << std::endl;
    }
}

int main(int argc, char *argv[]) {
    int task_id, num_procs;
    int array_size;
    int presorted = 0;
    if (argc < 3) {
        printf("\n Please provide the size of the array and then array type\n");
        return 0;
    }
    array_size = atoi(argv[1]);
    presorted = atoi(argv[2]);
    /* Define Caliper region names */
    const char* main_comp = "main_comp";
    const char* data_init = "data_init_runtime";
    const char* correctness_check = "correctness_check";
    const char* comm = "comm";
    const char* comm_small = "comm_small";
    const char* comm_large = "comm_large";
    const char* comp = "comp";
    const char* comp_small = "comp_small";
    const char* comp_large = "comp_large";
    const char* MPIx = "MPIx";

    adiak::init(NULL);
    adiak::launchdate();    // launch date of the job
    adiak::libraries();     // Libraries used
    adiak::cmdline();       // Command line used to launch the job
    adiak::clustername();   // Name of the cluster
    adiak::value("algorithm", "radix"); // The name of the algorithm you are using (e.g., "merge", "bitonic")
    adiak::value("programming_model", "mpi"); // e.g. "mpi"
    adiak::value("data_type", "int"); // The datatype of input elements (e.g., double, int, float)
    adiak::value("size_of_data_type", sizeof(int)); // sizeof(datatype) of input elements in bytes (e.g., 1, 2, 4)
    adiak::value("input_size", array_size); // The number of elements in input dataset (1000)

    switch(presorted){
      case 0:
        adiak::value("input_type", "Random"); 
      break;
      case 1:
        adiak::value("input_type", "Sorted");
      break;
      case 2:
        adiak::value("input_type", "ReverseSorted");
      break;
      case 3:
        adiak::value("input_type", "1_perc_perturbed");
      break;
      default:
        adiak::value("input_type", "Random"); // For sorting, this would be choices: ("Sorted", "ReverseSorted", "Random", "1_perc_perturbed")
    }

    adiak::value("scalability", "weak"); // The scalability of your algorithm. choices: ("strong", "weak")
    adiak::value("group_num", "14"); // The number of your group (integer, e.g., 1, 10)
    adiak::value("implementation_source", "online, ai, handwitten"); // Where you got the source code of your algorithm. choices: ("online", "ai", "handwritten").



    cali::ConfigManager mgr;
    mgr.start();

    CALI_MARK_BEGIN(main_comp);

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &task_id);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    adiak::value("num_procs", num_procs); // The number of processors (MPI ranks)

    if (num_procs < 2) {
        if (task_id == MASTER) {
            printf("At least 2 processes are required (1 master + 1 worker).\n");
        }
        MPI_Finalize();
        return 0;
    }

    MPI_Comm worker_comm;
    MPI_Comm_split(MPI_COMM_WORLD, task_id == MASTER ? MPI_UNDEFINED : 1, task_id, &worker_comm);

    CALI_MARK_BEGIN(comp);
    CALI_MARK_BEGIN(comp_small);

    int base_length = array_size / (num_procs - 1);
    int remainder = array_size % (num_procs - 1);
    int length_for_local_array = (task_id <= remainder) ? base_length + 1 : base_length;
   
    CALI_MARK_END(comp_small);
     CALI_MARK_END(comp);

    std::vector<int> local_data;
    if (task_id != MASTER) {
        CALI_MARK_BEGIN(data_init);
        generateData(task_id, num_procs, array_size, length_for_local_array, local_data, presorted);
        CALI_MARK_END(data_init);
        // Send unsorted data to master to write to file
        CALI_MARK_BEGIN(comm);
        CALI_MARK_BEGIN(comm_large);
        MPI_Send(local_data.data(), length_for_local_array, MPI_INT, MASTER, 0, MPI_COMM_WORLD);
        CALI_MARK_END(comm_large);
        CALI_MARK_END(comm);
    }

    if (task_id != MASTER) {
        CALI_MARK_BEGIN(comp);
        CALI_MARK_BEGIN(comp_large);
        parallel_radix_sort(local_data, 8 * sizeof(int), worker_comm);
        CALI_MARK_END(comp_large);
        CALI_MARK_END(comp);

        // Send sorted data to master via MPI_COMM_WORLD
        CALI_MARK_BEGIN(comm);
        CALI_MARK_BEGIN(comm_large);
        int sorted_size = local_data.size();
        MPI_Send(&sorted_size, 1, MPI_INT, MASTER, 1, MPI_COMM_WORLD);
        MPI_Send(local_data.data(), sorted_size, MPI_INT, MASTER, 2, MPI_COMM_WORLD);
        CALI_MARK_END(comm_large);
        CALI_MARK_END(comm);
    }

    if (task_id == MASTER) {
        int total_size = 0;
        CALI_MARK_BEGIN(comm);
        CALI_MARK_BEGIN(comm_large);
        // Gather unsorted data from all workers
        std::vector<int> unsorted_data(array_size);
        int offset = 0;
        for (int i = 1; i < num_procs; ++i) {
            int recv_length = (i <= remainder) ? base_length + 1 : base_length;
            MPI_Recv(unsorted_data.data() + offset, recv_length, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            offset += recv_length;
        }

        // Write unsorted data to file
        writeDataToFile("unsortedArray.csv", unsorted_data);

        // Gather sorted data from all workers
        std::vector<int> sorted_data;
        for (int i = 1; i < num_procs; ++i) {
            int recv_length;
            MPI_Recv(&recv_length, 1, MPI_INT, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            std::vector<int> temp_data(recv_length);
            MPI_Recv(temp_data.data(), recv_length, MPI_INT, i, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            sorted_data.insert(sorted_data.end(), temp_data.begin(), temp_data.end());
        }

        // Write sorted data to file
        writeDataToFile("sortedArray.csv", sorted_data);
        CALI_MARK_END(comm_large);
        CALI_MARK_END(comm);
        CALI_MARK_BEGIN(correctness_check);
        // Check if the sorted array is actually sorted
        checkSorted(sorted_data, array_size);
        CALI_MARK_END(correctness_check);
    }

    if (worker_comm != MPI_COMM_NULL) {
        MPI_Comm_free(&worker_comm);
    }

    CALI_MARK_END(main_comp);
    mgr.stop();
    mgr.flush();

    MPI_Finalize();

    return 0;
}
