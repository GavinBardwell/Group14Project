# CSCE 435 Group project

## 0. Group number: 
14
## 1. Group members:
1. Gavin Bardwell
2. Tyson Long
3. Andrew Pribadi
4. Darwin White

## 2. Project topic
We will implement and analyze the effectiveness of four parallel sorting algorithms. In our project we will implement bitonic, sample, merge, and radix sorts. We will compare overall sorting times between processes, efficiency with limited number of cores, and compare and contrast various edge cases to find maximum and minimum times for all sorting algorithms. 
For primary communication we will utilize a text message group chat.    
### 2a. Brief project description (what algorithms will you be comparing and on what architectures)

- Bitonic Sort: Bitonic sort is a recursive sorting algorithm which sorts bitonic sequences by comparing and swapping sections of the of the array in a predefined order. A bitonic sequence is a sequence that is strictly increasing, then decreasing. At each stage of the sort a portion
of the sequence is swapped and then remerged into a larger portion of the sequence. Only arrays of size 2^n can be sorted.
- Sample Sort: Sample sort is a divide-and-conquer algorithm similar to how quicksort partitions its input into two parts at each step revolved around a singluar pivot value, but what makes sample sort unique is that it chooses a large amount of pivot values and partitions the rest of the elements on these pivots and sorts all these partitions.
- Merge Sort: Merge sort is a divide-and-conquer algorithm that sorts an array by splitting the array into halves until each sub-array contains a single element. Each sub-array is then put back together in a sorted order.
- Radix Sort: Radix sorting is a non-comparative sorting algorithm where numbers are placed into buckets based on singular digits going from least significant to most significant.

### 2b. Pseudocode for each parallel algorithm

- For MPI programs, include MPI calls you will use to coordinate between processes

# Pseudocode for MPI Parallel Bitonic Sort
- Bitonic Sort Pseudocode
- Inputs is your global array

1. Seed the array in the parent process
2. Scatter the values to the processes
3. Sort each process into a bitonic sequence
4. Send values between each partner process
5. Keep swapping until we reach a sorted array
6. Gather all the process values back into the master
7. Check for correctness and finalize

```
//Bitonic Merge
bitonicMerge(data, low, count, direction) {
    if (count > 1)
        int k = count / 2;
        for (int i = low; i < low + k; ++i)
            if ((data[i] > data[i + k]) == direction)
                std::swap(data[i], data[i + k]);
        bitonicMerge(data, low, k, direction);
        bitonicMerge(data, low + k, k, direction);
}

//Bitonic Sort (sequential on local data)
bitonicSort(data, low, count, direction) {
    if (count > 1)
        int k = count / 2;
        bitonicSort(data, low, k, true);  // Sort in ascending order
        bitonicSort(data, low + k, k, false);  // Sort in descending order
        bitonicMerge(data, low, count, direction);
}

main() {
    //Initialize array and MPI
    MPI_Init();

    //Getting the total processes and rank
    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    //Initializing the array to be sorted
    if (rank == 0)
        arr = random array;
    subarray(2^n / size);
    int localSize = 2^n / size;

    //Scatter the data from root to all processes
    MPI_Scatter(arr, localSize, MPI_INT, arr, subarray, MPI_INT, 0, MPI_COMM_WORLD);

    //Sort the local data using bitonic sort
    bitonicSort(subarray, 0, localSize, true);

    //Perform parallel bitonic sorting
    for (int phase = 1; phase <= size; ++phase)
        int partner = rank ^ (1 << (phase - 1));  //Find partner using bitwise XOR

        //Exchange data with partner process
        dataReceived(localSize);
        MPI_Sendrecv(subarray, localSize, MPI_INT, partner, 0,
                     dataReceived, localSize, MPI_INT, partner, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        //Merge received data with local data
        if (rank < partner)
            // Ascending order
            subarray.insert(subarray.end(), dataReceived.begin(), dataReceived.end());
            bitonicMerge(subarray, 0, subarray.size(), true);
            subarray.resize(localSize);  // Keep only first half
        else
            //Descending order
            subarray.insert(subarray.end(), dataReceived.begin(), dataReceived.end());
            bitonicMerge(subarray, 0, subarray.size(), false);
            subarray.resize(localSize);  // Keep only second half

    //Gather the sorted subarrays at root process
    MPI_Gather(subarray, localSize, MPI_INT, arr, localSize, MPI_INT, 0, MPI_COMM_WORLD);

    MPI_Finalize();
    return 0;
}
```
# Pseudocode for MPI Parallel Sample Sort
- Sample Sort Pseudocode 
- Inputs is your global array
1. Initialize MPI
2. Sample n * k elements from the unsorted array
3. Share these samples with every processor
4. Sort the samples and select k-th, 2*k-th, 3*k-th...n*k-th as pivots
5. Split the data into buckets according to pivots
6. Send each bucket to their respective processor
7. Sort each bucket in parallel
8. Gather all the sorted buckets merging at the root processor
9. Finalize MPI

```
main () {
    // Initialization 
    arr = input array 
    MPI_Init(); 

    int num_proc, rank; 
    MPI_COMM_RANK(MPI_COMM_WORLD, &rank) 
    MPI_COMM_SIZE(MPI_COMM_WORLD, &num_proc) size = arr / num_proc 

    // Distribute data 
    if (rank == 0) { 
        localArr = arr with 'size' amount of elements MPI_Scatter(arr, size, localArr) 
    } 

    // Local Sort on each Processor 
    sampleSort(localArr[rank])

    // Sampling 
    sample_size = num_proc - 1 samples = select_samples(localArr[rank], sample_size)

    // Gather samples on root 
    MPI_Gather(localArr, size, sortedArr) 
    if (rank == 0) { 
        sorted_samples = SampleSort(sortedArr) 
        pivots = Choose_Pivots(sorted_samples, num_proc - 1) 
    }

    // Broadcast MPI_Bcast(&pivots, size, MPI_INT, root, MPI_COMM_WORLD);

    // Redistribute data according to pivots 
    send_counts, recv_counts, send_displacements, recv_displacements = arr size num_proc 
    for(int i = 0; i < size; i++) { 
        for(int j = 0; j < num_proc; j++) { 
            if(localArr[rank][i] <= pivots[j]) 
                send_data_to_processor(j) 
        } 
    }

    // Perform All-to-All communication 
    MPI_Alltoall(send_counts, send_displacements, recv_counts, recv_displacements)

    // Local sort again after redistribution 
    sampleSort(localArr[rank])

    // Gather sorted subarrays 
    MPI_Gather(localArr[rank], size, gatherSortedArr)

    // Final merge at root processor 
    if (rank == 0) 
        sampleSort(gatherSortedArr)

    // Finalize MPI 
    MPI_Finalize();
}
```

# Pseudocode for MPI Parallel Merge Sort
- Merge Sort Pseudocode
- Inputs is your global array
1. Initialize MPI
2. Divide the array evenly among processes
3. Perform sequential merge sort on subarray on each process
4. Gather subarrays at the root process
5. Perform merge sort on combined array
6. Finalize MPI

```
main() {
    // Initialize an unsorted array (arr)
    arr = unsorted array;

    // Variables to track rank and number of processes
    int world_rank;
    int world_size;

    // Initialize MPI
    MPI_INIT();

    // Get current process rank
    MPI_COMM_RANK(MPI_COMM_WORLD, &world_rank);

    // Get total number of processes
    MPI_COMM_SIZE(MPI_COMM_WORLD, &world_size);

    // Divide array into equal-sized chunks
    int size = size_of_array / world_size;

    // Allocate space for each process's sub-array (subArr)
    subArr = allocate array of size 'size'

    // Scatter the array across processes
    MPI_Scatter(arr, size, subArr);

    // Each process performs mergeSort on its sub-array
    mergeSort(subArr);

    // Gather the sorted sub-arrays at root process (world_rank == 0)
    MPI_Gather(subArr, size, sorted_arr);

    // Root process merges the sorted sub-arrays into a final sorted array
    if(world_rank == 0) {
        mergeSort(sorted_arr);
    }

    // Finalize MPI
    MPI_Finalize();
}

mergeSort(arr) {
    // Base case: If array has only one element, it is already sorted
    if left < right {
        // Find the midpoint of the array
        mid = (left + right) / 2;

        // Recursively sort the left half of the array
        mergeSort(left half of arr);

        // Recursively sort the right half of the array
        mergeSort(right half of arr);

        // Merge the two sorted halves
        merge(arr, left, mid, right);
    }
}

merge(arr, left, mid, right) {
    // Allocate a temporary array to store the merged result
    tempArr = temporary array of size (right - left + 1)

    // Initialize pointers for the left and right halves
    left_pointer = left;
    right_pointer = mid + 1;
    temp_pointer = left;

    // While both halves have elements
    while left_pointer <= mid AND right_pointer <= right {
        if arr[left_pointer] <= arr[right_pointer] {
            // Add the smaller element from the left half to tempArr
            tempArr[temp_pointer] = arr[left_pointer];
            left_pointer++;
        } else {
            // Add the smaller element from the right half to tempArr
            tempArr[temp_pointer] = arr[right_pointer];
            right_pointer++;
        }
        temp_pointer++;
    }

    // Copy any remaining elements from the left half
    while left_pointer <= mid {
        tempArr[temp_pointer] = arr[left_pointer];
        left_pointer++;
        temp_pointer++;
    }

    // Copy any remaining elements from the right half
    while right_pointer <= right {
        tempArr[temp_pointer] = arr[right_pointer];
        right_pointer++;
        temp_pointer++;
    }

    // Copy the merged elements from tempArr back to arr
    for i = left to right {
        arr[i] = tempArr[i];
    }
}
```


# Pseudocode for MPI Parallel Radix Sort
- Radix Sort Pseudocode
- Inputs is your global array size, number of processes, and type of sorting
the algorithm goes as such
1. Generate your data in each thread
- this is then sent to the master where it writes it to "unSortedArray.csv"
2. perform radix sort
- seperate each thread into buckets of bits 1s and 0s
- send the 0s to the top and 1s after that
- repeat with the next bit
- send to the master thread
3. Verify that the radix sort worked
- this is done in the master thread
4. write the sorted data to "sortedArray.csv"
## Main Function
```
main() {
    // Initialize MPI environment
    MPI_Init();

    // Variables to track rank and number of processes
    int task_id;
    int num_procs;

    // Get current process rank
    MPI_Comm_rank(MPI_COMM_WORLD, &task_id);

    // Get total number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    // If task is master
    if (task_id == MASTER) {
        // Distribute data among workers
        for each worker (i from 1 to num_procs - 1) {
            // Calculate data chunk size and send data
            MPI_Send(data_chunk, size, MPI_INT, i, 0, MPI_COMM_WORLD);
        }

        // Receive unsorted data from workers and write to file
        for each worker (i from 1 to num_procs - 1) {
            MPI_Recv(unsorted_data, size, MPI_INT, i, 1, MPI_COMM_WORLD, &status);
            writeDataToFile("unsortedArray.csv", unsorted_data);
        }

        // Receive sorted data from workers and write to file
        for each worker (i from 1 to num_procs - 1) {
            MPI_Recv(sorted_data, size, MPI_INT, i, 2, MPI_COMM_WORLD, &status);
            writeDataToFile("sortedArray.csv", sorted_data);
        }

        // Check if sorted correctly
        checkSorted(sorted_data);
    }
    else {
        // Receive data from master
        MPI_Recv(local_data, size, MPI_INT, MASTER, 0, MPI_COMM_WORLD, &status);

        // Perform radix sort on local data
        parallel_radix_sort(local_data, max_bits, MPI_COMM_WORLD);

        // Send unsorted data to master
        MPI_Send(local_data, size, MPI_INT, MASTER, 1, MPI_COMM_WORLD);

        // Send sorted data to master
        MPI_Send(local_data, size, MPI_INT, MASTER, 2, MPI_COMM_WORLD);
    }

    // Finalize MPI environment
    MPI_Finalize();
}
```

## Radix Sort Function
```
parallel_radix_sort(local_data, max_bits, comm) {
    // Iterate over each bit from least significant to most significant
    for bit from 0 to max_bits - 1 {
        // Split data into zero and one buckets based on current bit
        zero_bucket = elements with bit 0;
        one_bucket = elements with bit 1;

        // Gather sizes of zero buckets from all processes
        MPI_Allgather(size of zero_bucket, global_zero_sizes);

        // Calculate displacements for gathering zero bucket data
        calculate_displacements(global_zero_sizes, zero_recv_displs);

        // Gather all zero bucket data from processes
        MPI_Allgatherv(zero_bucket, global_zero_sizes, zero_recv_displs, zero_recv_buffer);

        // Gather sizes of one buckets from all processes
        MPI_Allgather(size of one_bucket, global_one_sizes);

        // Calculate displacements for gathering one bucket data
        calculate_displacements(global_one_sizes, one_recv_displs);

        // Gather all one bucket data from processes
        MPI_Allgatherv(one_bucket, global_one_sizes, one_recv_displs, one_recv_buffer);

        // Merge zero and one buckets to update local data
        local_data = zero_recv_buffer + one_recv_buffer;

        // Synchronize all processes before moving to next bit
        MPI_Barrier(comm);
    }
}
```

## Radix Sort Helper Functions
### `checkSorted(data)`
```
checkSorted(data) {
    // Iterate through data to verify sorting
    for i from 1 to size of data - 1 {
        if data[i] < data[i - 1] {
            print "Array is NOT sorted correctly.";
            return;
        }
    }
    print "Array is sorted correctly.";
}
```
### `writeDataToFile(filename, data)`
```
writeDataToFile(filename, data) {
    // Open file in write mode
    open file with name filename;
    
    // Write each element of data to file
    for element in data {
        write element to file;
    }
    
    // Close the file
    close file;
}
```





### 2c. Evaluation plan - what and how will you measure and compare
We will be working through arrays of sizes 2^16, 2^18, 2^20, 2^22, 2^24, 2^26, 2^28. We will test the sorting speed of presorted, randomly sorted, reverse sorted, and 1% perturbed. 

We will also test the all of these with increasing processors in range 2, 4, 8 ,16, 32, 64, 128, 256, 512, and 1024. At the end we will have run 280 sorts one for each array size with each array type and each processor count. This will allow us to analyze and understand the advantages and disadvantages of all sorting algorithms tested.


### 3a. Caliper instrumentation

Bitonic Sort

```
1.507 main
├─ 0.000 MPI_Init
├─ 0.027 data_init_runtime
├─ 0.877 comm
│  ├─ 0.015 MPI_Scatter
│  └─ 0.862 comp
│     ├─ 0.823 comp_large
│     └─ 0.001 comm_small
│        └─ 0.001 MPI_Sendrecv
├─ 0.000 MPI_Finalize
├─ 0.015 ve@
├─ 0.000 MPI_Initialized
├─ 0.000 MPI_Finalized
└─ 0.008 MPI_Comm_dup
```
Sample Sort
```
65.972 main
├─ 0.000 MPI_Init
├─ 2.780 data_init_runtime
├─ 4.374 comm
│  ├─ 2.782 MPI_Bcast
│  ├─ 0.378 comm_small
│  │  └─ 0.378 MPI_Scatter
│  ├─ 0.634 MPI_Gather
│  └─ 0.580 MPI_Alltoall
├─ 53.208 comp
├─ 0.000 MPI_Bcast
├─ 0.704 correctness_check
├─ 0.000 MPI_Finalize
├─ 0.000 MPI_Initialized
├─ 0.000 MPI_Finalized
└─ 1.667 MPI_Comm_dup

```

Merge Sort

```
96.017 main
├─ 0.000 MPI_Init
├─ 5.792 data_init_runtime
├─ 24.045 comm
│  ├─ 0.808 comm_large
│  │  ├─ 0.437 MPI_Scatter
│  │  └─ 0.371 MPI_Gather
│  └─ 23.237 MPI_Barrier
├─ 42.682 comp
│  └─ 42.682 comp_large
├─ 0.711 correctness_check
├─ 0.000 MPI_Finalize
├─ 0.000 MPI_Initialized
├─ 0.000 MPI_Finalized
└─ 0.000 MPI_Comm_dup
```

Radix Sort 
```
0.349 main_comp
├─ 0.000 MPI_Comm_free
├─ 0.002 MPI_Comm_split
├─ 0.181 comm
│  └─ 0.181 comm_large
│     ├─ 0.292 MPI_Recv
│     └─ 0.001 MPI_Send
├─ 0.159 comp
│  ├─ 0.318 comp_large
│  │  ├─ 0.001 MPI_Allgather
│  │  ├─ 0.003 MPI_Allgatherv
│  │  └─ 0.000 MPI_Barrier
│  └─ 0.000 comp_small
├─ 0.002 correctness_check
└─ 0.010 data_init_runtime
```
### 3b. Collect Metadata

We collect the following metadata for our implementations: the launch date of the job, the libraries used, the command line used to launch the job, the name of the cluster, the name of the algorithm you are using, the programming model, he datatype of input elements, the size of the datatype, the number of elements in input dataset, the input type of array, the number of processors, the scalability of our algorithms, the number of your group, and where we got the source code of our algorithm.

### **See the `Builds/` directory to find the correct Caliper configurations to get the performance metrics.** They will show up in the `Thicket.dataframe` when the Caliper file is read into Thicket.
## 4. Performance evaluation

Include detailed analysis of computation performance, communication performance. 
Include figures and explanation of your analysis.

### 4a. Vary the following parameters
For input_size's:
- 2^16, 2^18, 2^20, 2^22, 2^24, 2^26, 2^28

For input_type's:
- Sorted, Random, Reverse sorted, 1%perturbed

MPI: num_procs:
- 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024

This should result in 4x7x10=280 Caliper files for your MPI experiments.

### 4b. Hints for performance analysis

To automate running a set of experiments, parameterize your program.

- input_type: "Sorted" could generate a sorted input to pass into your algorithms
- algorithm: You can have a switch statement that calls the different algorithms and sets the Adiak variables accordingly
- num_procs: How many MPI ranks you are using

When your program works with these parameters, you can write a shell script 
that will run a for loop over the parameters above (e.g., on 64 processors, 
perform runs that invoke algorithm2 for Sorted, ReverseSorted, and Random data).  

### 4c. You should measure the following performance metrics
- `Time`
    - Min time/rank
    - Max time/rank
    - Avg time/rank
    - Total time
    - Variance time/rank


## 5. Presentation
Plots for the presentation should be as follows:
- For each implementation:
    - For each of comp_large, comm, and main:
        - Strong scaling plots for each input_size with lines for input_type (7 plots - 4 lines each)
        - Strong scaling speedup plot for each input_type (4 plots)
        - Weak scaling plots for each input_type (4 plots)

Analyze these plots and choose a subset to present and explain in your presentation.

## 6. Final Report
Submit a zip named `TeamX.zip` where `X` is your team number. The zip should contain the following files:
- Algorithms: Directory of source code of your algorithms.
- Data: All `.cali` files used to generate the plots seperated by algorithm/implementation.
- Jupyter notebook: The Jupyter notebook(s) used to generate the plots for the report.
- Report.md
