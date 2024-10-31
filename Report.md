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

![image](https://github.com/user-attachments/assets/ee269bf6-8b71-43b7-a66e-11b986a33fd1)
![image](https://github.com/user-attachments/assets/03d35489-846b-4e3a-8231-98a8fee0bee3)
![image](https://github.com/user-attachments/assets/115add6e-1164-4a26-ad38-82fa72952661)
![image](https://github.com/user-attachments/assets/cb794724-9d5e-4af1-acc1-0f09dd4f3b9f)
![image](https://github.com/user-attachments/assets/9d1d5ec6-513b-4a53-88c0-a3c8d1a3b739)
![image](https://github.com/user-attachments/assets/706f8156-ad30-4f8b-9b1c-20861fcc73dc)
![image](https://github.com/user-attachments/assets/416642dc-c2d6-4a65-8ab6-f933454b3d55)

The communication graphs effectively show the power of parallelizing a bitnoic sort on more and more processors. As we increase the number
of processors, the communication time decreases. Although we are increasing the number of messages sent, we are decreasing the amount of
data sent between each message. This causes a noticeable amount of speedup as we add more processors. There is data missing here, but I don't
believe the runs failing was caused by the main communication portion of the algorithm. It's also worth mentioning that on the smaller input
sizes the communication overhead caused by more processors was enough to increase the overall communication time.

![image](https://github.com/user-attachments/assets/3bf6cacb-d2aa-46e8-81ef-41375173025d)
![image](https://github.com/user-attachments/assets/e233c95f-a024-43cf-b648-38ce27adc034)
![image](https://github.com/user-attachments/assets/1b4ddc46-e007-4911-a81a-3db9bdcf4bd4)
![image](https://github.com/user-attachments/assets/46ca1c32-0e31-49f3-a4f6-87d63d167f43)
![image](https://github.com/user-attachments/assets/d735f123-213d-4933-a8b5-2f66b5220bbb)
![image](https://github.com/user-attachments/assets/bf70089f-043e-4f0d-b6cc-a3fbe630d3fc)
![image](https://github.com/user-attachments/assets/7153ea4b-5c3b-4bdd-9f3e-e78516f5a63d)

One thing to note about these graphs is that I had a smaller communication region nested within the computation portion of my algorithm. This
caused the smaller input sizes to actually increase in computation time as we added more processors similar to the starting communication graphs.
However, for all the other input sizes, the graph decreases in computation time as we add more processors.

![image](https://github.com/user-attachments/assets/c1c8ccfe-03e4-4d36-8b83-293de94481bc)
![image](https://github.com/user-attachments/assets/6fb19de7-8d13-465e-b542-39b1b944ade5)
![image](https://github.com/user-attachments/assets/e5c4d14e-4c91-4579-9bd2-b5e2d85da1c4)
![image](https://github.com/user-attachments/assets/c3c01c48-3769-4807-aeca-9b5464cc7417)
![image](https://github.com/user-attachments/assets/143cf0c3-0a6a-4a4a-b42b-e6464e09a7f8)
![image](https://github.com/user-attachments/assets/04442fc4-2938-4384-8e75-7c3fa560c053)
![image](https://github.com/user-attachments/assets/6746f2c2-308f-490c-ad4b-f4712edbee3e)

These are the communication region inside of the large computation section. They stay fairly constant and are slightly increased by the
large input sizes due to more data being sent. Overall, they remain low though.

![image](https://github.com/user-attachments/assets/66bb05d0-50f6-4247-8126-ba74349e7257)
![image](https://github.com/user-attachments/assets/d7236449-04fd-41c6-b6fa-67b644b1f28e)
![image](https://github.com/user-attachments/assets/8d94ac61-4b71-403e-867f-9e4227a31054)
![image](https://github.com/user-attachments/assets/6669b6fb-018d-4d60-bcd6-d912d0812520)
![image](https://github.com/user-attachments/assets/1e31f04d-369f-4db6-b57f-638ebcfb1053)
![image](https://github.com/user-attachments/assets/f20ba64a-f8f7-4620-8bf5-8e513b8da3a3)
![image](https://github.com/user-attachments/assets/05f64c95-3a44-4847-86d9-fd85d49d30d5)

When looking at the main speedup, you'll notice that the speedup started by decreasing on the smaller input sizes and then begin slowly
increasing as the we started using larger and large input sizes. The decreasing on the smaller input sizes was definitely caused by
communication overhead caused by the larger number of processors. Eventually, the program time got so long that the speedup caused by having
more processors outweighed the communication overhead. Therefore, the speedup slowly began increasing with input size increases.

![image](https://github.com/user-attachments/assets/0d8511de-dca7-4c64-b7b7-52c11da59091)
![image](https://github.com/user-attachments/assets/a7dd837b-9329-4920-8125-2eb333a34621)
![image](https://github.com/user-attachments/assets/5f4552ee-448a-4d79-a8f0-9486735340c0)
![image](https://github.com/user-attachments/assets/32df3ffe-5cee-4549-a8e9-ac00a3585448)
![image](https://github.com/user-attachments/assets/cda3a841-50fc-47c5-9d5a-51ca70c1fcfd)
![image](https://github.com/user-attachments/assets/ec22205a-0acf-4d74-8229-86ce561cedfe)
![image](https://github.com/user-attachments/assets/9d6126a8-d334-4a45-a22a-7f68393ed352)

Communication experiences a very similar speedup as to the main program speedup. At the beginning, the overhead caused by more processors
slowed it down, but with higher input sizes, it bagan experiencing rapid speedup.

![image](https://github.com/user-attachments/assets/f08425b0-d193-432a-9e5f-c8a5afa803f6)
![image](https://github.com/user-attachments/assets/629ec2cf-12ae-489f-8aea-4a5798b85fc4)
![image](https://github.com/user-attachments/assets/b5f5359e-a82f-417e-a561-273d1e187089)
![image](https://github.com/user-attachments/assets/1586bded-c537-431b-b043-d5f3a08394aa)
![image](https://github.com/user-attachments/assets/b5926c3b-7226-493c-baa5-d5494b184990)
![image](https://github.com/user-attachments/assets/d75781f9-78f1-4801-ad26-7ba4f993b374)
![image](https://github.com/user-attachments/assets/2eaa48cb-0ee5-4a75-8af9-a6367f6e10ac)

Once again, the large computation section experienced slow down on the smaller input sizes. This is likely due to communication overhead.
However, there was a significant amount of speedup as we added more processors to the large input sizes. Even though we are missing some data
in the biggest input sizes, I believe that the speedup would have either continued. There may have been a point at which it plateaued, but
the data I do have shows significant speedup up to the processor sizes the did run.

![image](https://github.com/user-attachments/assets/f4b2e929-6f8b-4f8a-b5ff-6e70915ce0ff)
![image](https://github.com/user-attachments/assets/cfb2e3c2-40cd-4128-8eca-85d0280f7814)
![image](https://github.com/user-attachments/assets/c6eee448-914c-4cfe-a6cf-819f21b58f71)
![image](https://github.com/user-attachments/assets/69423de5-75f8-49af-aa35-d3c62d6d0bcb)
![image](https://github.com/user-attachments/assets/de1580a1-df2f-4e3c-bc12-8bd799b9ec8a)
![image](https://github.com/user-attachments/assets/7e703b98-5b17-47ed-8e83-653570819206)
![image](https://github.com/user-attachments/assets/d4fa34a3-b764-4a5d-837e-13869c9a24a6)

Finally, the nested communication region is the same pattern once again. No speedup on the smaller input sizes due to communication overhead,
then significant speedup on the higher input sizes.

![image](https://github.com/user-attachments/assets/de3c6d34-b58e-45b0-b7e7-853761681fd2)
![image](https://github.com/user-attachments/assets/9d7275cb-cf3d-473f-88f4-902189225534)
![image](https://github.com/user-attachments/assets/afaae7e3-5cad-4b7b-80c0-746611cce6bf)
![image](https://github.com/user-attachments/assets/981fde14-2974-46ad-9e55-5e19d3c0372b)

For all 4 of the weak scaling plots, we see decreases in time across the board. Execution time starts high on the smaller number of processors,
but as we add more and more, the time gets lower and lower. One thing to notice is that there appears to be a certain point at which the 
execution time does not get any lower. For all of the graphs, that appears to be at 64 processors. This could be a limitation of my
algorithm implementation, or the bitonic sort itself. It's possible that with even higher input sizes, we still would have been able to
see decreases in execution time past 64 processors.

One thing you may have noticed throughout all the graphs was the missing data points on the higher input sizes. I was only able to
create 209 Caliper files from my algorithm, simply because a lot of the large input sizes did not finish running in reasonable amount of time.
I hardcapped every run of the algorithm at 30 minutes, so it's possible they would have finished with more time. I know this was most likely a
problem with my implementation. This could have been a problem with my large computation section although I find that unlikely. The one thing
I believe it to be was my correctness check at the end. After I finish computing the sort, I used an MPI_Gather to grab all the elements back
to the master to check the correctness of the array. I believe this is what caused so many of my runs to time out. Running a 2^28 array sequentially just takes too much time, and the communication overhead to do so may have caused issues.


**Sample Sort**
Note: All of the following Sample Sort graphs are limited to 64 processors as the cali files above 64 were not able to be processed by thicket correctly.
![image](https://github.com/user-attachments/assets/2aff0234-b8d3-4171-921e-8f156fdc87b3)
![image](https://github.com/user-attachments/assets/a96a633b-8cad-43a0-a0f5-47b7ee072e25)
![image](https://github.com/user-attachments/assets/c132bc85-3d5a-4428-bfc5-9c31f2ceed3f)
![image](https://github.com/user-attachments/assets/35ddb257-4763-4681-bdeb-78e88d5ad79d)
![image](https://github.com/user-attachments/assets/7f7c3694-8a70-4968-a376-f1f7d35cf22e)
![image](https://github.com/user-attachments/assets/8d0dbc81-1e63-40ed-9724-06f0d1673db1)
![image](https://github.com/user-attachments/assets/4469d9ef-b96e-4b68-9613-18a4d92f4e9a)

Observing these main strong scaling graphs, processing time seems to increase as the amount of processors increase up until 2^24, at which point the execution time is higher with less processors and decreases as the amount of processors increase. This is likely due to smaller input sizes result in wasted time through communication and synchronization overhead while larger input sizes utilize this time better. Overall, most of the graphs seem to taper off around 64 processors, indicating that adding more processors wouldn't make the algorithm any more efficient and it would be more efficient to look for ways to improve the algorithm itself rather than indefinitely scaling up the amount of processors.

![image](https://github.com/user-attachments/assets/d2f8ed68-11c9-471e-9f79-54efdbfac3b5)
![image](https://github.com/user-attachments/assets/0ba78029-9bc3-47ee-a1dd-d7df9ec954a5)
![image](https://github.com/user-attachments/assets/d2133a83-8e5a-4cc2-ba6a-801d02f96a90)
![image](https://github.com/user-attachments/assets/ff4a5502-60d6-4c14-a585-dc396ed76c80)
![image](https://github.com/user-attachments/assets/2eb1cf30-38be-45b2-a0e9-4492200233d4)
![image](https://github.com/user-attachments/assets/7666f9b2-9cb7-4f37-b3dc-d802d787a992)
![image](https://github.com/user-attachments/assets/005d433b-bb6b-48a2-9b55-4175339b4d2c)

For the comm strong scaling graphs, 2^16 is the only outlier with a very low time except for the reverse sorted arrays causing a huge spike which could just be nondeterministic. For each input size after that, they tend to look pretty similar with the graphs almost looking like a pyramid, and the perturbed arrays tend to have a large spike in time at 32 processors, and something to note is that starting from 2^22 and up input sizes, the random array types tend to have the highest time compared to the other array types. This indicates that non-uniform data access patterns increase communication costs greatly compared to other array types. Sorted and Reverse Sorted arrays tend to have the lowest communication overhead, suggesting better scalability for these array types.

![image](https://github.com/user-attachments/assets/80150cb2-1f9a-4114-94c6-1268590bbc66)
![image](https://github.com/user-attachments/assets/a1c7dfdd-484c-469e-be66-7b5aae0d9459)
![image](https://github.com/user-attachments/assets/ca3f95d9-63b8-40df-8514-daee0399b5f9)
![image](https://github.com/user-attachments/assets/18e0bdee-d596-462b-b269-09c7d41e6ede)
![image](https://github.com/user-attachments/assets/ec05870c-dfd4-473a-9032-6dfeb7dabdcf)
![image](https://github.com/user-attachments/assets/3d15adec-a840-42a0-9f02-ebf6dd2d10fa)
![image](https://github.com/user-attachments/assets/e8e00905-94b4-4b15-a17e-e568c930fe01)

For these comp large strong scaling graphs, 2^16 is again the only outlier however it makes sense as the lower input size could result in wasted/inefficient computation time as processor counts increase due to specific array types such as random, sorted, or perturbed. For all the input types larger than 2^16, the graphs are very consistent where as the amount of processors increase, the computation time decreases which aligns with the goal of parallelizing these algorithms of splitting up the workload would increase efficiency of the algorithm itself.

![image](https://github.com/user-attachments/assets/b99037ae-4589-47cd-8539-ee8b9237ab59)
![image](https://github.com/user-attachments/assets/8e56b810-4a1e-481f-b528-475b8ccd3ef3)
![image](https://github.com/user-attachments/assets/00f2a61e-6e50-4d16-87d6-3c32979f5f43)
![image](https://github.com/user-attachments/assets/79d657d0-858d-4d88-96f8-79bc34a47ad6)
![image](https://github.com/user-attachments/assets/2d7e0314-db00-483a-b622-8fa072de63fd)
![image](https://github.com/user-attachments/assets/bc2e6e3d-8878-4c95-8bde-6789e57545d9)
![image](https://github.com/user-attachments/assets/5d5557cc-955e-47ff-8caf-25f789c54519)

Looking at the main strong scaling speedup plots, speedup tends to almost always decline as processors increase from every input size lower than 2^26, which is likely due to communication and synchronization overhead. Despite this, 2^26 and 2^28 input types have a solid increase in speedup which show that larger input types benefit more from parallelization.

![image](https://github.com/user-attachments/assets/67cb36f1-55da-4662-abca-89ec01912b79)
![image](https://github.com/user-attachments/assets/b4c224cb-e348-48e3-ad27-858d66552564)
![image](https://github.com/user-attachments/assets/9cb2d018-a9eb-4402-947d-815ad8145e1d)
![image](https://github.com/user-attachments/assets/839b1b78-5a8b-4c94-9570-23f060d1f61f)
![image](https://github.com/user-attachments/assets/f22e9596-6863-4f26-889b-3a5cec56aeec)
![image](https://github.com/user-attachments/assets/1b65df29-4270-4f8d-b224-9de49cebb5b2)
![image](https://github.com/user-attachments/assets/3b7c446a-572e-4871-abbe-ce2ff5860d55)

In the comm strong scaling speedup plots, speedup almost always decreases as the amount of processors increase which is correct as there is always a larger amount of communication overhead with the increase in processors. Some of these graphs spike back up at 64 processors, which I cannot exactly tell if it is nondeterministic or not, but this could be due to these specific array types being sorted could help in communication with larger processor counts, as the random array type in these plots does not spike up as much as the other array types.

![image](https://github.com/user-attachments/assets/952ffc0d-a5a0-4698-8222-ff64e88397fd)
![image](https://github.com/user-attachments/assets/303e39a9-428d-473e-a08a-a63b49387204)
![image](https://github.com/user-attachments/assets/2af3b92f-3db0-491a-a99f-532671cddc6a)
![image](https://github.com/user-attachments/assets/d2ac4320-7f3e-47b8-a9c1-08b9cafe7de5)
![image](https://github.com/user-attachments/assets/b9b8e823-7d25-40ab-a982-c31fed413ffd)
![image](https://github.com/user-attachments/assets/b8a97b37-6669-4402-a506-eda7930bacdc)
![image](https://github.com/user-attachments/assets/c1f96ef9-3a5d-44b9-9139-94fd9e2320a9)

For the comp large strong scaling speedup plots, speedup almost always increases as processor amount increases which is correct as the increase in processor amount reduces computational load and would increase efficiency in these computations. The only outlier to this would be the 2^16 input type which is understandable as the smaller input could just be too inefficient with too many processors as it would take more time communicating than actual computation.

![image](https://github.com/user-attachments/assets/cd489118-c2d5-41d0-85ce-23d68a5c10b6)
For weak scaling main, the graph has a large drop in execution time from 2 to 4 processors, and afterwards remains very constant as processor count increases. This shows that the main doesn't have much issue in workload as processors increase.

![image](https://github.com/user-attachments/assets/395ebbd6-35bb-4c7b-ab22-80a456eff35d)
For the weak scaling comm graph, the time seems to increase as the processor count increases which aligns with the idea of increased communication overhead as there are more processors that the implementation has to communicate with. This is highlighted in the fact that the random array type has the largest execution time due to the unorganized input compared to other input types.

![image](https://github.com/user-attachments/assets/91f8cb6c-44b5-4014-95f6-e829885f7ad9)
For weak scaling comp large graph, the time drops tremendously from 2 to 4 processors and then stays low as processor count increases, showing the algorithm having to perform less work as processor counts increase due to the balanced workload. 

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

Strong Scaling Comm
![image](Radix_Sort/radix_images/comm_input_size_65536.png)
![image](Radix_Sort/radix_images/comm_input_size_262144.png)
![image](Radix_Sort/radix_images/comm_input_size_1048576.png)
![image](Radix_Sort/radix_images/comm_input_size_4194304.png)
![image](Radix_Sort/radix_images/comm_input_size_16777216.png)
![image](Radix_Sort/radix_images/comm_input_size_67108864.png)
![image](Radix_Sort/radix_images/comm_input_size_268435456.png)
Reviewing the communication graphs we an see the communication times dip dramatically. This does not make sense. Especially because We are adding more processes. I believe there is an issue with my MPI call resulting in errent times for communication. Going off the belief that this is correct we can assume communication decreasing means we are becoming more efficient with our communication with less data being switched around.

Speed up comm
![image](Radix_Sort/radix_images/comm_speedup_input_size_65536.png)
![image](Radix_Sort/radix_images/comm_speedup_input_size_262144.png)
![image](Radix_Sort/radix_images/comm_speedup_input_size_1048576.png)
![image](Radix_Sort/radix_images/comm_speedup_input_size_4194304.png)
![image](Radix_Sort/radix_images/comm_speedup_input_size_16777216.png)
![image](Radix_Sort/radix_images/comm_speedup_input_size_67108864.png)
![image](Radix_Sort/radix_images/comm_speedup_input_size_268435456.png)

Our speed up for communication is very irregular in our graph. It increases speed up each section however tends to plateau around the 128 process count. This could be the limit of efficiency so we would need more optimizations.

Weak Scaling Comm
![image](Radix_Sort/radix_images/comm_weak_scaling.png)
In ouyr weak scaling graph we see that the time for each process to communicate decreases as you add processes. Going back to our strong scaling this still does not make sense as you would think that as you add more processe your communication time would increase. 


Strong Scaling comp
![image](Radix_Sort/radix_images/comp_large_input_size_65536.png)
![image](Radix_Sort/radix_images/comp_large_input_size_262144.png)
![image](Radix_Sort/radix_images/comp_large_input_size_1048576.png)
![image](Radix_Sort/radix_images/comp_large_input_size_4194304.png)
![image](Radix_Sort/radix_images/comp_large_input_size_16777216.png)
![image](Radix_Sort/radix_images/comp_large_input_size_67108864.png)
![image](Radix_Sort/radix_images/comp_large_input_size_268435456.png)
The time decrease for computation also decreases as you add processes. Thius seems to flatten out and then crashes to near zero after 32 processes. As you add processes for larger array sizes the code is not able to complete it showing inefficiencient use of memory and needs for optimizaitions. 

Speed up comp
![image](Radix_Sort/radix_images/comp_large_speedup_input_size_65536.png)
![image](Radix_Sort/radix_images/comp_large_speedup_input_size_262144.png)
![image](Radix_Sort/radix_images/comp_large_speedup_input_size_1048576.png)
![image](Radix_Sort/radix_images/comp_large_speedup_input_size_4194304.png)
![image](Radix_Sort/radix_images/comp_large_speedup_input_size_16777216.png)
![image](Radix_Sort/radix_images/comp_large_speedup_input_size_67108864.png)
![image](Radix_Sort/radix_images/comp_large_speedup_input_size_268435456.png)

In our speed up graphs we can see a near linear speed up in our processes as you increase. This comes after 32 processes. This may come from the fact we are now using more than 1 node in Grace or could be some other issue. There seems to be more performance to be able to extract as the speed up was not 0 yet. 

Weak scaling Comp
![image](Radix_Sort/radix_images/comp_large_weak_scaling.png)

In our weak scaling we see that adding more processes is beneficial up to some point. While there may be more performance to achieve with larger process sizes 64 process count seems like the most optimal. It drops dramatically before hand but afterwards the somputation stays about the same. 

Strong Scaling main
![image](/Radix_Sort/radix_image/main_comp_input_size_65536.png)
![image](/Radix_Sort/radix_images/main_comp_input_size_262144.png)
![image](/Radix_Sort/radix_images/main_comp_input_size_1048576.png)
![image](/Radix_Sort/radix_images/main_comp_input_size_4194304.png)
![image](/Radix_Sort/radix_images/main_comp_input_size_16777216.png)
![image](/Radix_Sort/radix_images/main_comp_input_size_67108864.png)
![image](/Radix_Sort/radix_images/main_comp_input_size_268435456.png)

Looking over our strong scaling main graphs we can see how adding more processes for larger array sizes becomes more efficient. At smaller array sizes our processes may not be able to optimize the entire use of the array and too much communication was happening compared to what is needed. 

These graphs do decrease dramatically as you add processes at larger array sizes. However at some point the processes cant hold all the info from the arrays so crashes. If better space management was done from a code side this would become more efficient.

Speed up main
![image](Radix_Sort/radix_images/main_comp_speedup_input_size_65536.png)
![image](Radix_Sort/radix_images/main_comp_speedup_input_size_262144.png)
![image](Radix_Sort/radix_images/main_comp_speedup_input_size_1048576.png)
![image](Radix_Sort/radix_images/main_comp_speedup_input_size_4194304.png)
![image](Radix_Sort/radix_images/main_comp_speedup_input_size_16777216.png)
![image](Radix_Sort/radix_images/main_comp_speedup_input_size_67108864.png)
![image](Radix_Sort/radix_images/main_comp_speedup_input_size_268435456.png)

At our lower array sizes the speed up seems to die out and not be efficient. However after 2^20 the speed up stays constant but dips at the end for process sizes. At the last 2 array sizes we are not able to go past 8 processes due to bad communications. In general we have fine speed up but need to have better overall communication.

Weak scaling main
![image](/Radix_Sort/radix_images/main_comp_weak_scaling.png)

The speed up via weak scaling seems quite good. It decreases dramatically until around 32 processes. We need to have better communication to allow for more speed up and for it not hit its limit so early. 


**overall**

![image](TotalTimeCombo.png)

In our analysis of the various algorithms we can se that radix sort is very inefficient on few processes. As you increase with processes it becomes more efficient and levels out with the rest of the sorting algorithms. 

Our implementation of merge is consistent and adding processes does not yield better performance due to increase commnucation sizes. 

Our implementation of sample and bitonic are also not efficient on larger process sizes however were both better than radix sort and small process sizes. This shows that when using smaller servers use these instead of radix.

![image](speedUpCombo.png)

In our comparable speed up we see that radix has the most dramatic improvement. This is because it is so bad compared to the rest of the sorting algorithms that it has the most room to improve on its process time.

Merge and sort has almost no speed up. This can be due to the extra overhead found from the adding of more communication channels.

Bitonic has a little speed up but due to inconsistencies has issues at high process counts.

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
