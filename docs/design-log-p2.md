# Project 2 Design Log

## Conversation Growth

For the Conversation class, I chose a growth factor of 2 for the dynamically allocated array. The Conversation starts with a capacity of 0. When the first Message is added, the capacity becomes 1. After that, whenever the array becomes full, the capacity is doubled. This means that the capacity grows as 1, 2, 4, 8, 16, and so on. When a larger array is needed, a new array is allocated, the existing Messages are copied into the new array, and the old array is deleted.

I chose this method because it allows most calls to append() to be completed without allocating a new array. Normally, a new Message can simply be placed at the next available index, which takes O(1) time. An append that causes the array to grow is more expensive because all of the existing Messages have to be copied.

Even though some individual calls to append() take O(n), the average, or amortized, cost is O(1). For example, to reach a capacity of 16, the previous reallocations copy 1 + 2 + 4 + 8 = 15 Messages. More generally, the total number of Messages copied during growth follows a geometric series:

1 + 2 + 4 + ... + n/2 < n

Therefore, the total amount of copying needed over n insertions is O(n). Dividing this total work across the n calls to append() gives an amortized cost of O(1) for each insertion.

## Rule of Five

The Conversation class owns a dynamically allocated Message array, so I implemented the Rule of Five to manage this memory safely. The destructor uses delete[] to release the allocated array when the Conversation is destroyed.

For copying, the copy constructor creates a new Message array with the same capacity as the original Conversation. It then copies each Message from the original array into the new array. The copy assignment operator performs a similar process after first checking for self-assignment. This creates a deep copy, meaning that two Conversation objects do not share the same dynamically allocated array. Changing or destroying one Conversation therefore does not affect the memory owned by the other.

For moving, the move constructor transfers the data pointer, size, and capacity from the original object to the new object instead of copying every Message. The original object's pointer is then set to nullptr and its size and capacity are set to 0. The move assignment operator works similarly, except it first deletes the memory currently owned by the destination object. Resetting the moved-from object is important because it prevents two Conversation objects from trying to delete the same memory.

## Sentinel Scanner and Bounded Memory

The SentinelScanner must detect the sentinel even when it is divided between multiple chunks. I used the pending_ string to temporarily store characters that could be the beginning of the sentinel. For this project, the sentinel is `<|end_conversation|>`, but the scanner stores the sentinel as a string so that it can work with the sentinel passed to its constructor.

The scanner combines pending_ with each new chunk and checks whether the complete sentinel exists. If it finds the sentinel, it returns only the safe text that appears before it. If there is not a complete sentinel, the scanner checks whether the end of the received text could still be the beginning of the sentinel. Only that possible partial match is stored in pending_.

The important bound is that pending_ can never need to contain a complete sentinel. If the sentinel has length L, a possible unfinished match can have at most L - 1 characters. Once L matching characters have been received, the complete sentinel has been found and the scanner reports it instead of storing it in pending_. Therefore:

pending_.size() <= sentinel_.size() - 1

This means the amount of memory required by pending_ is bounded by the length of the sentinel instead of increasing with the total amount of text processed. The bounded-memory test verifies this behavior while processing a large stream one byte at a time.

## What I Would Change

If I were designing the project again, I would plan and write more of the tests while implementing each class instead of waiting until most of the implementation was finished. Writing the tests earlier would have made it easier to check each part of the Conversation and SentinelScanner as I added functionality. In particular, testing the scanner with the sentinel split at different positions helped me better understand why pending_ was necessary. I would also think through the edge cases for memory management and chunk boundaries earlier in the design process. This would make debugging more organized and help catch problems closer to when they were introduced.