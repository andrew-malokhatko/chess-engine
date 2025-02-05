#pragma once
#include <array>

namespace Chess::Sort
{
    template <typename T, std::size_t N>
    void Quicksort(std::array<T, N>& values, std::array<int, N>& scores, int low, int high);

    template <typename T, std::size_t N>
    int Partition(std::array<T, N>& values, std::array<int, N>& scores, int low, int high);

    template <typename T, std::size_t N>
    void Quicksort(std::array<T, N>& values, std::array<int, N>& scores, int low, int high)
    {
        if (low < high)
        {
            int pivotIndex = Partition(values, scores, low, high);
            Quicksort(values, scores, low, pivotIndex - 1);
            Quicksort(values, scores, pivotIndex + 1, high);
        }
    }

    template <typename T, std::size_t N>
    int Partition(std::array<T, N>& values, std::array<int, N>& scores, int low, int high)
    {
        int pivotScore = scores[high];
        int i = low - 1;

        for (int j = low; j < high; j++)
        {
            if (scores[j] > pivotScore)
            {
                i++;
                std::swap(values[i], values[j]);
                std::swap(scores[i], scores[j]);
            }
        }
        std::swap(values[i + 1], values[high]);
        std::swap(scores[i + 1], scores[high]);

        return i + 1;
    }
}