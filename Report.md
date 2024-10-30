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
- : ing is a non-comparative sorting algorithm where numbers are placed into buckets based on singular digits going from least significant to most significant.

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


# Pseudocode for MPI Parallel 
-  Pseudocode
- Inputs is your global array size, number of processes, and type of sorting
the algorithm goes as such
1. Generate your data in each thread
- this is then sent to the master where it writes it to "unSortedArray.csv"
2. perform 
- seperate each thread into buckets of bits 1s and 0s
- send the 0s to the top and 1s after that
- repeat with the next bit
- send to the master thread
3. Verify that the  worked
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

        // Perform  on local data
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

##  Function
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

##  Helper Functions
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

**Bitonic Sort**

![image](https://github.com/user-attachments/assets/5464bdf0-2fca-4230-b327-68a1ea80e860)
he main size increase mainly due to the communication requirements.

![image](https://github.com/user-attachments/assets/176a6bd3-a33d-4f5c-96be-048119cc681c)
It's clear that as we increase the number of processors, the communication requirements increase. This is obvious, as we are increasing the messages sent and received by all processes when we add more processors.

![image](https://github.com/user-attachments/assets/776c881d-451e-4e85-aa28-52f9092f645b)
The calculation increases slightly as we add more processors, but it's important to note that this includes a section of communication. This means the calculation only increase due to communication increases.

I had a significant amount of trouble generating caliper files for the other graphs; however, the implementation is sound, and the other graphs will be generated quickly moving forward.

**Sample Sort**
![image](https://github.com/user-attachments/assets/b3536b67-39d5-4489-aa98-0ace6313c4c3)
Looking at this main graph, it is clear that a sorted array drastically increases the time of the sort in comparison to a random array.
![image](https://github.com/user-attachments/assets/3f7280c2-6d87-4d61-a948-05d94a01ce1f)
For the comm graph, it stays pretty consistent times in the different array types. This is likely just due to the low amount of processors so they don't have to compete for resources.
![image](https://github.com/user-attachments/assets/a84480b7-4608-437f-982e-c7159fbec546)

![image](https://github.com/user-attachments/assets/a1875e8f-824d-4eb0-b566-6905d90b08fb)

![image](https://github.com/user-attachments/assets/53a47842-7e8d-40b2-80a3-b79c76172d49)

For the small computation regions, it seems like the time is just barely larger when an array is sorted in comparison to random, but in the large computation region this is flipped and the time is barely larger when the array is random.


**Merge Sort**

![image](https://github.com/user-attachments/assets/21118255-0b7c-430b-ae64-293636164e49)
![image](https://github.com/user-attachments/assets/93c44f81-5b40-4f97-876f-17b24f3af3ec)
![image](https://github.com/user-attachments/assets/6e5eac42-e1a0-4ec2-a610-6eca85a44399)
![image](https://github.com/user-attachments/assets/d7f9dbe4-2d71-4680-84ab-fa0eb36ff457)
![image](https://github.com/user-attachments/assets/ea8392a8-218e-45aa-bda7-ff9e5355d33c)
![image](https://github.com/user-attachments/assets/3ecc65e4-9ea4-47b6-85f7-677ea3e55c04)
![image](https://github.com/user-attachments/assets/50b70b16-a146-4775-a951-d12400f99021)

For main strong scaling, there appears to be a general trend of increasing time as the number of processes increases until we get to an array size of 2^24. For input sizes of 2^24, 2^26, and 2^28, there appears to be a sudden spike in time at a random number of processes. It then levels out to what it was before. This appears to happen at random for some input types. This pattern is not consistent.

The pattern of increasing time as the number of processes increases is most likely due to increasing communication overhead. As the number of processes increases, more communication needs to happen between the processes. The patter of random spikes in time at random places for different input types is most likely due to congestion as the many processes compete for resources.

![image](https://github.com/user-attachments/assets/dddcd6cf-62d9-4fba-b396-b7c5fd5f6f14)
![image](https://github.com/user-attachments/assets/8a087564-934f-4fef-bf02-0062d4b3b3f6)
![image](https://github.com/user-attachments/assets/e0bce2e9-d9bb-430f-9487-8419d28cd4f8)
![image](https://github.com/user-attachments/assets/181bf875-60e4-4b2b-9c93-7dff876a9c1d)
![image](https://github.com/user-attachments/assets/67c61288-5499-4269-833a-ab90ec312d65)
![image](https://github.com/user-attachments/assets/d75530aa-f50a-4c53-a483-6ae619ba1301)
![image](https://github.com/user-attachments/assets/fb903c9e-846b-4dcd-8f2e-66b688f3fdbe)

For comm strong scaling, there appears to be a random spikes in time at random places for all different input types. This is most likely due to congestion as the many processes compete for resources. These random spikes are likely nondeterministic.


![image](https://github.com/user-attachments/assets/7b302f60-c450-4280-8a9d-54c8bb123905)
![image](https://github.com/user-attachments/assets/cc370d74-85e0-4bab-89ac-c9a1b0b69c76)
![image](https://github.com/user-attachments/assets/543849ed-215b-4e0c-828c-6a7fc5178269)
![image](https://github.com/user-attachments/assets/7ee621bf-182f-457f-ace9-f20d63019703)
![image](https://github.com/user-attachments/assets/a8172ffb-c1b1-40ef-923f-aaa3deac82b5)
![image](https://github.com/user-attachments/assets/0639a0f8-8d28-4816-9e5d-7aadf10f8e4c)
![image](https://github.com/user-attachments/assets/ae4f98c4-21c6-47fe-b8d8-ed5dacb4949b)

For comp large strong scaling, there appears to be a trend of decreasing time as the number of processes increases. This is expected because as the number of processes increases, the workload is distributed across the many processes. Therefore, there is less work for each individual process to compute.

![image](https://github.com/user-attachments/assets/9a9e899d-f441-4829-b369-697c05e17a31)
![image](https://github.com/user-attachments/assets/e3f80bbb-d225-43a8-83e6-d75988d4f730)
![image](https://github.com/user-attachments/assets/c93af298-3b73-4880-87cc-0238c27f08e3)
![image](https://github.com/user-attachments/assets/60627f6a-b059-4f0f-94cd-849666247e47)
![image](https://github.com/user-attachments/assets/031d9c64-29bb-4af0-86a7-23a1dd3d68a0)
![image](https://github.com/user-attachments/assets/5d048f4e-9834-4fcd-9c5d-1acc31627ec6)
![image](https://github.com/user-attachments/assets/4d23ddb0-8237-48a1-8fe0-1384d001cfae)

For main strong scaling speed up, sequential merge sort seems faster for smaller input sizes. This is likely due to communication overhead. At the end of merge sort in parallel, all of the processes send their individual subarrays back to the master process, then one final merge sort must be performed. The cost of the individual processes communicating with each other does not justify doing merge sort in parallel for smaller input sizes. Specifically, we start seeing an improvement from doing merge sort in parallel at an input size of 2^24, but it becomes very clear that there is an improvement from performing merge in parallel with an input size of 2^28.

![image](https://github.com/user-attachments/assets/56ce6662-9e7f-40a7-82f8-979b62cced6b)
![image](https://github.com/user-attachments/assets/f3cdd5f2-f353-4c55-adb4-889b8cbaee67)
![image](https://github.com/user-attachments/assets/468378f4-6bfa-4f38-be1d-cb4e98b62adf)
![image](https://github.com/user-attachments/assets/1f7b13b5-b29b-4b88-9877-7efd06852abf)
![image](https://github.com/user-attachments/assets/ddd3ad14-bdd9-4c4c-9bc6-6f31a6b9c5aa)
![image](https://github.com/user-attachments/assets/b130834c-065a-485a-9415-be7940d2b9ae)
![image](https://github.com/user-attachments/assets/c1e44a61-d5bf-4298-9b8d-00577e43cddf)

For comm strong scaling speed up, a decreasing speed up graph is expected. This is because there is no communication overhead for a single process. There is no need for data transfer between multiple processes. However, as we increase the number of processes, this means more processes need to communicate with each other. Particularly for merge sort, each process is performing merge sort on a subarray of the original input. As all of these subarrays get sent back to the master process to perform one final merge sort, a lot of communication overhead takes place. This is what results in the decreasing graphs for comm strong scaling speed up.

![image](https://github.com/user-attachments/assets/33b45563-bad7-4c99-8055-177814a088a0)
![image](https://github.com/user-attachments/assets/420f6eaa-c616-44cf-bc25-986ccb79cf91)
![image](https://github.com/user-attachments/assets/a23bc927-2d2a-42e2-9a04-1b30ec210006)
![image](https://github.com/user-attachments/assets/55c48852-f4ab-49a6-879b-99192e886194)
![image](https://github.com/user-attachments/assets/bee5c557-caa2-4b45-a6a5-60ced4dd9d3e)
![image](https://github.com/user-attachments/assets/3d4b3978-5289-4d26-9dd4-8855b3dd9ec3)
![image](https://github.com/user-attachments/assets/d35f1ef2-66d3-4561-881b-eaf9a7fb9d8c)

For comp large strong scaling speed up, all of the graphs are increasing for each input size. This is because the work is divided evenly among the many processes. For a single process, the computation time depends mostly on the input size. If the work is divided among more processes, the work load for each individual process is less. This decreases the overall computation time for each process since each process is working with a smaller subarray. This is what results in the increasing graph as the number of processes increases.

![image](https://github.com/user-attachments/assets/3c92a664-66e8-48d9-a628-8c6f5dc1ccca)
For main weak scaling, the graph appears to be relatively constant. This suggests that merge sort scales well with an increasing workload. Each process handles its portion of the workload without significant overhead.

![image](https://github.com/user-attachments/assets/be8b0cbe-53f6-4495-87ef-ae3187c6d3ab)
For comm weak scaling, the increasing graph as the number of processes increases and the input size increases suggests that the communication overhead is also growing, interfering with the overall execution time of the merge sort algorithm. However, with a slighly logarithmic curve, this will likely become less of a problem for input sizes greater than 2^28.

![image](https://github.com/user-attachments/assets/c995de7d-ee8d-4fda-97dc-d55846849f1a)
For comp large weak scaling, the graph appears to be decreasing. This suggests that there is efficient workload distribution, resulting in less work for each process as we increase the number of processes and input size.



**Radix Sort**
Currently there are issues regarding the metadata that has made analysis extremely difficult. This comes from a mislabeling of the number of processors and matrix size. Radix sort has ran 210/280 of the needed files with most errors coming at the 2^26 and 2^28 inputs and at the 512 and 1024 processor size. This shows that is room for improvement on networking and optomization of parameters. There is not enough communication being done and there is too much of a reliance on the master process.
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
