/*
Created by @PieroCastillo on 2025-09-30
*/
#ifndef LETTUCE_CORE_TABLE_GROUP_HPP
#define LETTUCE_CORE_TABLE_GROUP_HPP

// standard headers
#include <optional>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

// project headers
#include "common.hpp"

namespace Lettuce::Core
{
    /*
    TableGroup is a collection of tables, allocate in one VkDeviceMemory & VkBuffer.
    Each DeviceTable has a set of columns, all elements of the same column have the same size.

    |----------------Allocation----------------|
    |-----------------Buffer-------------------|
    |  Table 0   |   Table 1   |   Table 2   | ... |   Table N   |
    Each Table has its own offset in the buffer
    */

    struct PendingCopy
    { 
        void* data;
        uint32_t size;
    };    
    
    template <typename T>
    class TableColumnView
    {
        void EnqueueCopyRange(uint32_t startIndex, uint32_t count, const T* data);
    };

    class DeviceTable
    {
        friend class TableGroup;
    private:
        std::vector<std::string> m_columnNames;
        std::vector<uint64_t>    m_columnOffsets;
        std::vector<uint64_t>    m_columnSizes;
        uint32_t                 m_maxCount;

        std::vector<PendingCopy> m_pendingCopies;
    public:
        template <typename T>
        TableColumnView<T> GetColumn(const std::string& name);
    };

    struct TableCreateInfo 
    {
        std::string name;
        std::vector<std::pair<std::string, uint64_t>> columns;
        uint32_t maxCount;
    };

    /// @brief  Information to create a table group
    struct TableGroupCreateInfo
    {
        std::vector<TableCreateInfo> tables;
        bool supportsStorage;
        bool supportsIndirectDraws;
        bool supportsAdvancedIndirectDraws;
        bool supportsExecutionGraphs;
    };

    class TableGroup
    {
        uint64_t m_address;
        uint64_t m_size;
        std::unordered_map<std::string, std::unique_ptr<DeviceTable>> m_tables;
        std::vector<uint64_t> m_tableOffsets;

        VkCommandPool m_cmdPool;
        VkCommandBuffer m_cmd;
        VkQueue m_transferQueue;
    public:
        VkDevice       m_device;
        VkDeviceMemory m_memory;
        VkBuffer       m_buffer;

        void Create(const IDevice& device, const TableGroupCreateInfo& createInfo);
        void Release();

        void FlushCopies();

        inline DeviceTable& GetTable(const std::string& name)
        {
            return *m_tables.at(name);
        }
        inline uint32_t GetTableCount() 
        {
            return m_tables.size();
        }
        inline uint32_t GetRawSize()
        {
            return m_size;    
        }
    };
}

#endif // LETTUCE_CORE_TABLE_GROUP_HPP