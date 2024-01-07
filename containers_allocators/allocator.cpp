#include <iostream>
#include <set>

class SmallAllocator {
private:
    struct MemoryBlock {
        MemoryBlock(size_t start_index, size_t block_size) : start_(-static_cast<int>(start_index)), block_size_(block_size) {}

        [[nodiscard]] size_t GetStart() const {
            return static_cast<size_t>(-start_);
        }

        [[nodiscard]] size_t GetEnd() const {
            return GetStart() + block_size_;
        }

        int start_;
        size_t block_size_;
    };

public:
    void *Alloc(unsigned int Size) {
        size_t current_index = first_free_index_;

        while (true) {
            const MemoryBlock new_block{current_index, Size};
            const size_t new_block_end = new_block.GetEnd();

            if (new_block_end >= MAX_BUFFER_SIZE) {
                break;
            }

            auto it = occupied_blocks_.lower_bound(new_block);
            bool can_be_occupied = it == occupied_blocks_.end() || it->GetEnd() <= current_index;

            if (can_be_occupied) {
                if (first_free_index_ == current_index) {
                    first_free_index_ = new_block_end;
                }
                occupied_blocks_.insert(new_block);
                return buffer_ + current_index;
            }

            current_index = it->GetEnd();
        }

        throw std::bad_alloc{};
    };

    void *ReAlloc(void *Pointer, unsigned int Size) {
        const size_t prev_block_size = find_occupied_block(Pointer)->block_size_;
        void *new_pointer = Alloc(Size);
        std::memcpy(new_pointer, Pointer, std::min(static_cast<size_t>(Size), prev_block_size));
        Free(Pointer);
        return new_pointer;
    };

    void Free(void *Pointer) {
        auto it = find_occupied_block(Pointer);
        first_free_index_ = std::min(it->GetStart(), first_free_index_);
        occupied_blocks_.erase(it);
    };

private:
    std::set<MemoryBlock>::iterator find_occupied_block(void *pointer) {
        const size_t start = reinterpret_cast<std::uintptr_t>(pointer) - reinterpret_cast<std::uintptr_t>(&buffer_);
        const MemoryBlock block{start, 1};
        return occupied_blocks_.lower_bound(block);
    }

    static bool memory_blocks_comparator(const MemoryBlock& first, const MemoryBlock& second) {
        return first.start_ < second.start_;
    }

private:
    static constexpr size_t MAX_BUFFER_SIZE = 262144;

    char buffer_[MAX_BUFFER_SIZE]{};
    size_t first_free_index_ = 0;

    std::set<MemoryBlock, decltype(memory_blocks_comparator)*> occupied_blocks_{&memory_blocks_comparator};
};

int main() {
    SmallAllocator A1;
    int * A1_P1 = (int *) A1.Alloc(sizeof(int));
    A1_P1 = (int *) A1.ReAlloc(A1_P1, 2 * sizeof(int));
    A1.Free(A1_P1);
    SmallAllocator A2;
    int * A2_P1 = (int *) A2.Alloc(10 * sizeof(int));
    for(unsigned int i = 0; i < 10; i++) A2_P1[i] = i;
    for(unsigned int i = 0; i < 10; i++) if(A2_P1[i] != i) std::cout << "ERROR 1" << std::endl;
    int * A2_P2 = (int *) A2.Alloc(10 * sizeof(int));
    for(unsigned int i = 0; i < 10; i++) A2_P2[i] = -1;
    for(unsigned int i = 0; i < 10; i++) if(A2_P1[i] != i) std::cout << "ERROR 2" << std::endl;
    for(unsigned int i = 0; i < 10; i++) if(A2_P2[i] != -1) std::cout << "ERROR 3" << std::endl;
    A2_P1 = (int *) A2.ReAlloc(A2_P1, 20 * sizeof(int));
    for(unsigned int i = 10; i < 20; i++) A2_P1[i] = i;
    for(unsigned int i = 0; i < 20; i++) if(A2_P1[i] != i) std::cout << "ERROR 4" << std::endl;
    for(unsigned int i = 0; i < 10; i++) if(A2_P2[i] != -1) std::cout << "ERROR 5" << std::endl;
    A2_P1 = (int *) A2.ReAlloc(A2_P1, 5 * sizeof(int));
    for(unsigned int i = 0; i < 5; i++) if(A2_P1[i] != i) std::cout << "ERROR 6" << std::endl;
    for(unsigned int i = 0; i < 10; i++) if(A2_P2[i] != -1) std::cout << "ERROR 7" << std::endl;
    A2.Free(A2_P1);
    A2.Free(A2_P2);
    return 0;
}