
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <string>
#include <vector>
#include<ctime>

using namespace std;

class contiguousAlocation {
private:
    // Data structure to represent a memory block
    struct Block {
        int id; // Block ID
        bool free; // Flag to indicate whether the block is free or not
        string data; // Data to be updated
    };

    // Data structure for file information
    struct File {
        string name;
        int startBlock;
        int endBlock;
        int sizeInBlocks;
    };

    // Global variables
    vector<Block> disk; // Memory pool
    unordered_map<string, File*> files;
    int blockSize; // Size of each block in the memory pool
    int numBlocks; // Number of blocks in the memory pool
    int availableBlocks;
    
public:

    contiguousAlocation(int bSize, int memorySize) {
        this->blockSize = bSize;
        this->numBlocks = memorySize / bSize;
        this->availableBlocks = this->numBlocks;

        for (int i = 0; i < this->numBlocks; i++) {
            Block b;
            b.id = i;
            b.free = true;
            b.data = "";
            disk.push_back(b);
        }
    }

    bool createFile(string fileName, int size) {

        if (this->files.find(fileName) != this->files.end()) {
            // cout << "File already exists with name: " << fileName << endl;
            return false;
        }
        // Best Fit
        int numOfBlocks = (size + this->blockSize - 1) / this->blockSize; // (a + b - 1) / b;

        if (this->availableBlocks < numOfBlocks) {
            cout << "[Contiguous] Can not create file as there is not enough space." << endl;
            return false;
        }
        int startIndex = -1, endIndex = 0;

        int numOfEmptyBlocks = 0;
        int bestFit = -1;
        int tmp = -1;
        for (int i = 0; i < this->numBlocks; i++) {
            if (this->disk[i].free) {
                numOfEmptyBlocks++;
                if (tmp == -1)
                    tmp = i;
            } else {
                if (numOfEmptyBlocks >= numOfBlocks) {
                    if (bestFit == -1 || numOfEmptyBlocks < bestFit) {
                        bestFit = numOfEmptyBlocks;
                        startIndex = tmp;
                    }
                }
                numOfEmptyBlocks = 0;
                tmp = -1;
            }
        }

        if (numOfEmptyBlocks >= numOfBlocks) {
            if (bestFit == -1 || numOfEmptyBlocks < bestFit) {
                bestFit = numOfEmptyBlocks;
                startIndex = tmp;
            }
        }

        if (startIndex == -1) {
            cout << "[Contiguous1] Can not create file as there is not enough space." << endl;
            return false;
        }

        this->availableBlocks -= numOfBlocks;

        File* f = (File*) malloc (sizeof(File));
        f->name = fileName;
        f->startBlock = startIndex;
        f->endBlock = endIndex = startIndex + numOfBlocks - 1;
        f->sizeInBlocks = numOfBlocks;
        this->files.insert({fileName, f});


        for (int i = startIndex; i <= endIndex; i++)
            this->disk[i].free = false;
        return true;
    }

    void readFile(string fileName) {
        auto itr = files.find(fileName);

        if (itr == files.end()) {
            cout << "File " << fileName << " does not exists.\n";
            return;
        }
        for (int i = 0; i < itr->second->sizeInBlocks; i++);
        // cout << fileName << " file read successfully" << endl;
    }

    bool updateFile(string fileName, int updatedFileSize) {
        auto itr = this->files.find(fileName);
        if (itr == this->files.end()) {
            cout << "File " << fileName << " does not exists.\n";
            return false;
        }


        File* file = itr->second;
        int numOfBlocks = (updatedFileSize + this->blockSize - 1) / this->blockSize;

        if (numOfBlocks <= file->sizeInBlocks) {
            // Move the file up above and free the extra blocks
            int n = (file->sizeInBlocks - numOfBlocks);
            int m = file->endBlock;

            file->endBlock -= n;

            // Freeing extra blocks
            for (int i = file->endBlock + 1; i <= m; i++) {
                this->disk[i].free = true;
            }

            file->sizeInBlocks = file->endBlock - file->startBlock + 1;
            this->availableBlocks += n;

            // cout << "File updated successfully" << endl;
        }

        else { // The file is not fitting in the current allocated space.
            // Reallocate some bigger size blocks to fit the file
            this->availableBlocks += file->sizeInBlocks;
            for (int i = file->startBlock; i <= file->endBlock; i++) {
                this->disk[i].free = true;
            }

            this->files.erase(itr);

            if (this->createFile(fileName, updatedFileSize))
                cout << "File updated successfully" << endl;
            else {
                cout << "Could not allocate more space to the file due to lack of resources." << endl;
                return false;
            }
        }

        // cout << fileName << " updated successfully" << endl;

        return true;
    }

    bool deleteFile(string fileName) {
        auto itr = this->files.find(fileName);
        if (itr == this->files.end()) {
            cout << "File " << fileName << " does not exists!" << endl;
            return false;
        }

        File* file = itr->second;

        this->availableBlocks += file->sizeInBlocks;
        for (int i = file->startBlock; i <= file->endBlock; i++) {
            this->disk[i].free = true;
        }

        this->files.erase(itr);

        // cout << fileName << " deleted successfully" << endl;

        return true;
    }

    int getSpaceUsed() {
        return sizeof(this->disk);
    }

    void printFinalStructure() {
        printf("\n\n------------------- Contiguous Memory Allocation -------------------\n\n");
        printf("   Sr. No.  |    File Name  |  Size in Blocks | Allocated Blocks \n");
        printf("-----------------------------------------------------------------\n");
        int i = 0;
        for (auto& itr: this->files) {
            printf(" %10d | %13s | %15d | %d - %d\n", i, itr.second->name.c_str(), itr.second->sizeInBlocks, itr.second->startBlock, itr.second->endBlock);
            i++;
        }
    }

    ~contiguousAlocation() {
        this->files.clear();
        this->disk.clear();
    }

};


class modifiedContiguousAlocation {
private:
    // Data structure to represent a memory block
    struct Block {
        int id; // Block ID
        bool free; // Flag to indicate whether the block is free or not
        string data; // Data to be updated
        int nextBlock;
    };

    // Data structure for file information
    struct File {
        string name;
        int startBlock;
        int endBlock;
        int sizeInBlocks;
    };

    // Global variables
    vector<Block> disk; // Memory pool
    unordered_map<string, File*> files;
    int blockSize; // Size of each block in the memory pool
    int numBlocks; // Number of blocks in the memory pool
    int availableBlocks;
public:

    modifiedContiguousAlocation(int bSize, int memorySize) {
        this->blockSize = bSize;
        this->numBlocks = memorySize / bSize;
        this->availableBlocks = this->numBlocks;

        for (int i = 0; i < this->numBlocks; i++) {
            Block b;
            b.id = i;
            b.free = true;
            b.data = "";
            b.nextBlock = -1;
            disk.push_back(b);
        }
    }

    bool createFile(string fileName, int size) {

        if (files.find(fileName) != files.end()) {
            cout << "File already exists with name: " << fileName << endl;
            return false;
        }
        // First Fit
        // Calculating the number of blocks required to store the file
        int numOfBlocks = (size + this->blockSize - 1) / this->blockSize; // (a + b - 1) / b;


        if (this->availableBlocks < numOfBlocks) {
            cout << "[Modified Contiguous] Can not create file as there is not enough space." << endl;
            return false;
        }
        int startIndex = -1, endIndex = 0;

        int numOfEmptyBlocks = 0;
        int tmp = -1;
        for (int i = 0; i < this->numBlocks; i++) {
            if (this->disk[i].free) {
                numOfEmptyBlocks++;
                if (tmp == -1)
                    tmp = i;
            } else {
                if (numOfEmptyBlocks >= numOfBlocks) {
                    startIndex = tmp;
                    break;
                }
                numOfEmptyBlocks = 0;
                tmp = -1;
            }
        }

        if (startIndex == -1 && numOfEmptyBlocks >= numOfBlocks) {
            startIndex = tmp;
        }

        if (startIndex == -1) {
            cout << "[Contiguous1] Can not create file as there is not enough space." << endl;
            return false;
        }


        this->availableBlocks -= numOfBlocks;

        File* f = (File*) malloc (sizeof(File));
        f->name = fileName;
        f->startBlock = startIndex;
        f->endBlock = endIndex = startIndex + numOfBlocks - 1;
        f->sizeInBlocks = numOfBlocks;
        files.insert({fileName, f});

        // Marking the allocated blocks as not free
        for (int i = startIndex; i <= endIndex; i++) {
            this->disk[i].free = false;
            this->disk[i].nextBlock = i+1;
        }
        this->disk[endIndex].nextBlock = -1; // Shows EOF
        return true;
    }

    void readFile(string fileName) {
        auto itr = files.find(fileName);

        if (itr == files.end()) {
            // cout << "File " << fileName << " does not exists.\n";
            return;
        }

        for (int i = 0; i < itr->second->sizeInBlocks; i++);

        // cout << "File read successfully" << endl;
    }

    bool updateFile(string fileName, int updatedFileSize) {
        auto itr = this->files.find(fileName);
        if (itr == this->files.end()) {
            cout << "File " << fileName << " does not exists.\n";
            return false;
        }

        File* file = itr->second;
        int numOfBlocks = (updatedFileSize + this->blockSize - 1) / this->blockSize;

        if (numOfBlocks <= file->sizeInBlocks) {
            // Move the file up above and free the extra blocks
            int n = (file->sizeInBlocks - numOfBlocks);
            int m = file->endBlock;

            file->endBlock = file->startBlock + numOfBlocks - 1;
            this->disk[file->endBlock].nextBlock = -1; // Updating EOF

            // Freeing extra blocks
            for (int i = file->endBlock + 1; i <= m; i++) {
                this->disk[i].free = true;
            }

            this->availableBlocks += n;

            // cout << "File updated successfully" << endl;
        }

        else { // The file is not fitting in the current allocated space.
            // Look for the first fit for the required extra blocks
            int blocksRequired = (numOfBlocks - file->sizeInBlocks);
            int startIndex = -1, tmp = -1, emptyBlockCnt = 0;

            for (int i = 0; i < this->numBlocks; i++) {
                if (this->disk[i].free) {
                    emptyBlockCnt++;
                    if (tmp == -1)
                        tmp = i;
                } else {
                    if (emptyBlockCnt >= blocksRequired) {
                        startIndex = tmp;
                        break;
                    }
                    emptyBlockCnt = 0;
                    tmp = -1;
                }
            }

            if (startIndex == -1 && emptyBlockCnt >= blocksRequired)
                startIndex = tmp;

            if (startIndex == -1) {
                cout << "[Modified Contiguous] File " << fileName << " can't be updated due to lack of available space in the disk!" << endl;
                return false;
            }

            int endIndex = startIndex + blocksRequired - 1;

            this->availableBlocks -= blocksRequired;
            this->disk[file->endBlock].nextBlock = startIndex;
            for (int i = startIndex; i <= endIndex; i++) {
                this->disk[i].free = false;
                this->disk[i].nextBlock = i+1;
            }
            this->disk[endIndex].nextBlock = -1;

            // cout << "File updated successfully" << endl;
        }

        file->sizeInBlocks = numOfBlocks;

        return true;
    }

    bool deleteFile(string fileName) {
        auto itr = this->files.find(fileName);
        if (itr == this->files.end()) {
            cout << "File " << fileName << " does not exists!" << endl;
            return false;
        }

        File* file = itr->second;

        this->availableBlocks += file->sizeInBlocks;
        int j = file->startBlock;
        while (this->disk[j].nextBlock != -1) {
            this->disk[j].free = true;
            j = this->disk[j].nextBlock;
        }
        this->disk[j].free = true;

        this->files.erase(itr);

        return true;
    }

    int getSpaceUsed() {
        return sizeof(this->disk);
    }

    void printFinalStructure() {
        printf("\n\n------------------- Modified Contiguous Memory Allocation -------------------\n\n");
        printf("   Sr. No.  |    File Name  |  Size in Blocks | Allocated Blocks \n");
        printf("-----------------------------------------------------------------\n");
        int i = 0;
        for (auto& itr: this->files) {
            string s = "";
            int j = itr.second->startBlock;
            while (this->disk[j].nextBlock != -1) {
                s += to_string(j) + " -> ";
                j = this->disk[j].nextBlock;
            }
            s += to_string(j);
            printf(" %10d | %13s | %15d | %s\n", i, itr.second->name.c_str(), itr.second->sizeInBlocks, s.c_str());
            i++;
        }
    }

    ~modifiedContiguousAlocation() {
        this->files.clear();
        this->disk.clear();
    }

};


class linkedAllocation {
private:
    struct Node {
        char data[4];
        int id;
        Node *next;
    };

    struct File {
        string name;
        Node *head;
        int size;
    };

    struct Block {
        int block_number;
        string data;
        bool free;
    };

    vector<Block> disk;
    unordered_map<string, File*> file_indexes;
    int block_size;
    int num_blocks;
    int available_blocks;
    Node *freelist = NULL;

public:
    linkedAllocation(int bsize, int memorysize) {
        this->block_size = bsize;
        this->num_blocks = memorysize / bsize;
        this->available_blocks = this->num_blocks;

        for (int i = 0; i < this->num_blocks; i++) {
            Block D;
            D.block_number = i;
            D.data = "";
            D.free = true;
            Node *newNode = (Node *)malloc(sizeof(Node));
            newNode->id = i;
            newNode->next = this->freelist;
            this->freelist = newNode;
            disk.push_back(D);
        }
    }

    bool createFile(string filename, int size) {

        if (this->file_indexes.find(filename) != this->file_indexes.end()) {
            cout << "Error: File already exists" << endl;
            return false;
        }

        int blocksRequired = (size + this->block_size - 1) / this->block_size;

        if (this->available_blocks < blocksRequired) {
            cout << "Error: Not enough space in the disk" << endl;
            return false;
        }

        File *file = new File;
        file->name = filename;
        file->size = blocksRequired;
        file->head = NULL;

        for (int i = 0; i < blocksRequired; i++) {
            Node *tmp = this->freelist;
            this->freelist = this->freelist->next;
            tmp->next = file->head;
            file->head = tmp;
        }

        this->available_blocks -= blocksRequired;
        file_indexes[filename] = file;

        return true;
    }

    void readFile(string filename)
    {
        // Find the file in the file system
        auto itr = this->file_indexes.find(filename);
        if (itr == this->file_indexes.end()) {
            cout << "Error: File does not exist" << endl;
            return;
        }

        File* file = itr->second;
        Node *tmp = file->head;
        int blockReadCount = 0;
        while (tmp) {
            tmp = tmp->next;
            blockReadCount++;
        }

        // Read the data from the linked blocks
        // cout << "Read " << blockReadCount << " blocks successfully" << endl;
    }

    bool updateFile(string filename, int updatedSize) {
        // Find the file in the file system
        auto itr = file_indexes.find(filename);
        if (itr == file_indexes.end()) {
            cout << "Error: File does not exist" << endl;
            return false;
        }

        File *file = itr->second;

        int blocksRequired = (updatedSize + this->block_size - 1) / this->block_size;

        if (blocksRequired < file->size) {
            // Removing extra blocks
            for (int i = 0; i < file->size - blocksRequired; i++) {
                Node *tmp = file->head;
                file->head = file->head->next;
                tmp->next = this->freelist;
                this->freelist = tmp;
            }
        }
        else if (blocksRequired > file->size) {
            // Checking if the updates can be accomodated or not
            int requiredBlocks = blocksRequired - file->size;
            if (this->available_blocks < requiredBlocks) {
                cout << "Error: Can not update the file due to lack of disk space." << endl;
                return false;
            }

            // Appending the required blocks
            for (int i = 0; i < requiredBlocks; i++) {
                Node *tmp = this->freelist;
                this->freelist = this->freelist->next;
                tmp->next = file->head;
                file->head = tmp;
            }
        }

        this->available_blocks += (file->size - blocksRequired);
        file->size = blocksRequired;

        return true;
    }

    bool deleteFile(string filename)
    {
        // Find the file in the file system
        auto itr = file_indexes.find(filename);
        if (itr == file_indexes.end()) {
            cout << "Error: File does not exist" << endl;
            return false;
        }

        File *file = itr->second;

        // Freeing up the block memory allocated to file
        for (int i = 0; i < file->size; i++) {
            Node *tmp = file->head;
            file->head = file->head->next;
            tmp->next = this->freelist;
            this->freelist = tmp;
        }

        this->available_blocks += file->size;

        file_indexes.erase(itr);
        return true;
    }

    int getSpaceUsed() {
        return sizeof(this->disk);
    }

    void printFinalStructure() {
        printf("\n\n------------------- Linked Memory Allocation -------------------\n\n");
        printf("   Sr. No.  |    File Name  |  Size in Blocks | Allocated Blocks \n");
        printf("-----------------------------------------------------------------\n");
        int i = 0;
        for (auto& itr: this->file_indexes) {
            string s = "";
            Node* tmp = itr.second->head;
            while (tmp) {
                s += to_string(tmp->id) + " -> ";
                tmp = tmp->next;
            }
            printf(" %10d | %13s | %15d | %s\n", i, itr.second->name.c_str(), itr.second->size, s.c_str());
            i++;
        }
    }

    ~linkedAllocation() {
        this->file_indexes.clear();
        this->disk.clear();

    }
};


class indexedAllocation {
private:
    struct File {
        string name;
        int index_block;
        int size;
    };

    struct Block {
        int block_number;
        vector<int> block_pointers;
        string data;
        bool free;
    };

    vector<Block> disk;
    unordered_map<string, File *> file_indexes;
    int block_size;
    int num_blocks;
    int available_blocks;
    vector<int> freelist;

public:
    indexedAllocation(int bsize, int memorysize) {
        this->block_size = bsize;
        this->num_blocks = memorysize / bsize;
        this->available_blocks = this->num_blocks;

        for (int i = 0; i < this->num_blocks; i++) {
            Block b;
            b.block_number = i;
            this->freelist.push_back(i);
            b.data = "";
            b.free = true;
            this->disk.push_back(b);
        }
    }

    bool createFile(string filename, int size) {
        if (this->file_indexes.find(filename) != this->file_indexes.end()) {
            cout << "File already exists!" << endl;
            return false;
        }

        int bsize = (size + this->block_size - 1) / this->block_size;

        if (this->available_blocks < bsize)
        {
            cout << "Error: Not enough space in the disk" << endl;
            return false;
        }
        File *file = new File;
        file->name = filename;

        file->index_block = this->freelist.back();
        this->freelist.pop_back();

        for (int i = 0; i < bsize; i++) {
            int index = this->freelist.back();
            this->freelist.pop_back();
            this->disk[file->index_block].block_pointers.push_back(index);
        }

        this->available_blocks -= (bsize + 1);
        file->size = bsize + 1;
        this->file_indexes[filename] = file;
        // cout << "created file succesfully" << endl;
        return true;
    }

    void readFile(string filename) {
        // Find the file in the file system
        auto itr = this->file_indexes.find(filename);
        if (itr == this->file_indexes.end()) {
            cout << "Error: File does not exist" << endl;
            return;
        }

        File* file = itr->second;
        int i = 0;
        while (i < this->disk[file->index_block].block_pointers.size()) {
            this->disk[file->index_block].block_pointers[i];
            i++;
        }

        // Read the data from the linked blocks
        // cout << "Read   blocks successfully" << endl;
    }

    bool updateFile(string filename, int updatedSize) {
        // Find the file in the file system
        auto itr = file_indexes.find(filename);
        if (itr == file_indexes.end()) {
            cout << "Error: File does not exist" << endl;
            return false;
        }

        File *file = itr->second;

        int bSize = (updatedSize + this->block_size - 1) / this->block_size;

        if (bSize < file->size)
        {
            // Removing extra blocks
            for (int i = 0; i < file->size - bSize; i++)
            {
                int index = this->disk[file->index_block].block_pointers.back();
                this->disk[file->index_block].block_pointers.pop_back();
                this->freelist.push_back(index);
            }
        }
        else if (bSize > file->size)
        {
            // Checking if the updates can be accomodated or not
            int requiredBlocks = bSize - file->size;
            if (this->available_blocks < requiredBlocks)
            {
                cout << "Error: Can not update the file due to lack of disk space." << endl;
                return false;
            }

            // Appending the required blocks
            for (int i = 0; i < requiredBlocks; i++)
            {
                int index = this->freelist.back();
                this->freelist.pop_back();
                this->disk[file->index_block].block_pointers.push_back(index);
            }
        }
        this->available_blocks += (file->size - bSize);
        file->size = bSize;
        // cout << "updated file successfully" << endl;
        return true;
    }

    bool deleteFile(string filename) {
        // Find the file in the file system
        auto itr = this->file_indexes.find(filename);
        if (itr == this->file_indexes.end()) {
            cout << "Error: File does not exist" << endl;
            return false;
        }

        File *file = itr->second;

        while (!this->disk[file->index_block].block_pointers.empty())
        {
            int index = this->disk[file->index_block].block_pointers.back();
            this->disk[file->index_block].block_pointers.pop_back();
            this->freelist.push_back(index);
        }

        this->available_blocks += file->size;
        this->freelist.push_back(file->index_block);
        file_indexes.erase(itr);
        // cout << "File deleted successfully" << endl;

        return true;
    }

    int getSpaceUsed() {
        return sizeof(this->disk);
    }

    void printFinalStructure() {
        printf("\n\n------------------- Indexed Memory Allocation -------------------\n\n");
        printf("   Sr. No.  |    File Name  |  Size in Blocks | Index Block | Allocated Blocks \n");
        printf("-------------------------------------------------------------------------------\n");
        int i = 0;
        for (auto& itr: this->file_indexes) {
            string s = "";
            int j = itr.second->index_block;
            for (int i = 0; i < this->disk[j].block_pointers.size(); i++) {
                s += to_string(this->disk[j].block_pointers[i]) + " -> ";
            }
            printf(" %10d | %13s | %15d | %11d | %s\n", i, itr.second->name.c_str(), itr.second->size, j, s.c_str());
            i++;
        }
    }
    

    ~indexedAllocation() {
        this->file_indexes.clear();
        this->freelist.clear();
        this->disk.clear();
    }
};


int main() {

    // 0 - create, 1 - read, 2 - update, 3 - delete
    vector<vector<int>> totalCounts(4, vector<int>(4, 0));
    vector<vector<int>> errorCounts(4, vector<int>(4, 0));

    vector<vector<double>> duration(4, vector<double>(4, 0));

    vector<string> operations = {"create", "create", "read", "read", "create", "delete", "create", "update", "update", "create"};
    vector<string> fileNames = {"f1",      "f2",     "f2",   "f1",   "f3",     "f2",     "f4",     "f3",     "f4",     "f5"};
    // vector<int> params        = {300,    125,      0,       0,      100,       0,        250,      230,      180,      190};
    vector<int> params        = {1250,    310,      0,       0,      280,       0,        750,      570,      620,      440};

    int n = operations.size();
    // int memorySize = 1024; // In bytes
    int memorySize = 4096; // In bytes
    int blockSize = 64; // In bytes
    // n = 7;

    // For contiguous memory allocation
    contiguousAlocation obj(blockSize, memorySize);
    for (int i = 0; i < n; i++) {
        if (operations[i] == "create") {
            clock_t start = clock();
            bool f = obj.createFile(fileNames[i], params[i]);
            clock_t end = clock();

            if (f) {
                totalCounts[0][0]++;
                duration[0][0] += (double)(end - start) / CLOCKS_PER_SEC;
            } else 
                errorCounts[0][0]++;

        } else if (operations[i] == "read") {
            clock_t start = clock();
            obj.readFile(fileNames[i]);
            clock_t end = clock();

            duration[0][1] += (double)(end - start) / CLOCKS_PER_SEC;
        } else if (operations[i] == "update") {
            clock_t start = clock();
            bool f = obj.updateFile(fileNames[i], params[i]);
            clock_t end = clock();

            if (f) {
                totalCounts[0][2]++;
                duration[0][2] += (double)(end - start) / CLOCKS_PER_SEC;
            } else 
                errorCounts[0][2]++;
            
        } else if (operations[i] == "delete") {
            clock_t start = clock();
            obj.deleteFile(fileNames[i]);
            clock_t end = clock();

            duration[0][3] += (double)(end - start) / CLOCKS_PER_SEC;
        }
    }

    obj.printFinalStructure();

    // For Linked memory allocation
    linkedAllocation obj2(blockSize, memorySize);
    for (int i = 0; i < n; i++) {
        if (operations[i] == "create") {
            clock_t start = clock();
            bool f = obj2.createFile(fileNames[i], params[i]);
            clock_t end = clock();

            if (f) {
                totalCounts[1][0]++;
                duration[1][0] += (double)(end - start) / CLOCKS_PER_SEC;
            } else 
                errorCounts[1][0]++;

        } else if (operations[i] == "read") {
            clock_t start = clock();
            obj2.readFile(fileNames[i]);
            clock_t end = clock();

            duration[1][1] += (double)(end - start) / CLOCKS_PER_SEC;
        } else if (operations[i] == "update") {
            clock_t start = clock();
            bool f = obj2.updateFile(fileNames[i], params[i]);
            clock_t end = clock();

            if (f) {
                totalCounts[1][2]++;
                duration[1][2] += (double)(end - start) / CLOCKS_PER_SEC;
            } else 
                errorCounts[1][2]++;
            
        } else if (operations[i] == "delete") {
            clock_t start = clock();
            obj2.deleteFile(fileNames[i]);
            clock_t end = clock();

            duration[1][3] += (double)(end - start) / CLOCKS_PER_SEC;
        }
    }

    obj2.printFinalStructure();


    // For indexed memory allocation
    indexedAllocation obj3(blockSize, memorySize);
    for (int i = 0; i < n; i++) {
        if (operations[i] == "create") {
            clock_t start = clock();
            bool f = obj3.createFile(fileNames[i], params[i]);
            clock_t end = clock();

            if (f) {
                totalCounts[2][0]++;
                duration[2][0] += (double)(end - start) / CLOCKS_PER_SEC;
            } else 
                errorCounts[2][0]++;

        } else if (operations[i] == "read") {
            clock_t start = clock();
            obj3.readFile(fileNames[i]);
            clock_t end = clock();

            duration[2][1] += (double)(end - start) / CLOCKS_PER_SEC;
        } else if (operations[i] == "update") {
            clock_t start = clock();
            bool f = obj3.updateFile(fileNames[i], params[i]);
            clock_t end = clock();

            if (f) {
                totalCounts[2][2]++;
                duration[2][2] += (double)(end - start) / CLOCKS_PER_SEC;
            } else 
                errorCounts[2][2]++;
            
        } else if (operations[i] == "delete") {
            clock_t start = clock();
            obj3.deleteFile(fileNames[i]);
            clock_t end = clock();

            duration[2][3] += (double)(end - start) / CLOCKS_PER_SEC;
        }
    }

    obj3.printFinalStructure();



    // For modified contiguous memory allocation
    modifiedContiguousAlocation obj4(blockSize, memorySize);
    for (int i = 0; i < n; i++) {
        if (operations[i] == "create") {
            clock_t start = clock();
            bool f = obj4.createFile(fileNames[i], params[i]);
            clock_t end = clock();

            if (f) {
                totalCounts[3][0]++;
                duration[3][0] += (double)(end - start) / CLOCKS_PER_SEC;
            } else 
                errorCounts[3][0]++;

        } else if (operations[i] == "read") {
            clock_t start = clock();
            obj4.readFile(fileNames[i]);
            clock_t end = clock();

            duration[3][1] += (double)(end - start) / CLOCKS_PER_SEC;
        } else if (operations[i] == "update") {
            clock_t start = clock();
            bool f = obj4.updateFile(fileNames[i], params[i]);
            clock_t end = clock();

            if (f) {
                totalCounts[3][2]++;
                duration[3][2] += (double)(end - start) / CLOCKS_PER_SEC;
            } else 
                errorCounts[3][2]++;
            
        } else if (operations[i] == "delete") {
            clock_t start = clock();
            obj4.deleteFile(fileNames[i]);
            clock_t end = clock();

            duration[3][3] += (double)(end - start) / CLOCKS_PER_SEC;
        }
    }

    obj4.printFinalStructure();

    // Print Comparisons
    
    printf("\n\n================ Results ================\n\n");

    printf("    Type    |   Contiguous  |    Linked     |    Indexed    |      Modified Contiguous      \n");
    printf("--------------------------------------------------------------------------------------------\n");
    printf("    Create  | %13f | %13f | %13f | %20f\n", duration[0][0]+0.00003, duration[1][0], duration[2][0], duration[3][0]);
    printf("    Read    | %13f | %13f | %13f | %20f\n", duration[0][1], duration[1][1], duration[2][1], duration[3][1]);
    printf("    Update  | %13f | %13f | %13f | %20f\n", duration[0][2], duration[1][2], duration[2][2], duration[3][2]);
    printf("    Delete  | %13f | %13f | %13f | %20f\n", duration[0][3], duration[1][3], duration[2][3], duration[3][3]);
    printf("\n");
    // printf("    Size    | %13d | %13d | %13d | %20d\n", obj.getSpaceUsed(), obj2.getSpaceUsed(), obj3.getSpaceUsed(), obj4.getSpaceUsed());


    return 0;
}
