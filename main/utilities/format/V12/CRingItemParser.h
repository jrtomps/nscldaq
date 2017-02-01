#ifndef DAQ_V12_CRINGITEMPARSER_H
#define DAQ_V12_CRINGITEMPARSER_H

#include <ByteOrder.h>
#include <V12/DataFormat.h>
#include <V12/CRingItem.h>
#include <V12/CRingItemFactory.h>
#include <V12/CCompoundRingItem.h>
#include <utility>
#include <cstdint>

namespace DAQ {
namespace V12 {
namespace Parser {

bool isCompound(uint32_t type);

template<class ByteIterator>
bool mustSwap(ByteIterator beg, ByteIterator end) {
    return (*beg==0 && *(beg+1)==0);
}



template<class ByteIterator>
void peekHeaderType(ByteIterator beg, ByteIterator end, uint32_t& size, uint32_t& type, bool& swapRequired)
{

    if (beg+8 > end) {
        throw std::runtime_error("DAQ::V12::Parser::peekHeaderType() insufficient data provided");
    }

    swapRequired = mustSwap(beg+4, beg+8);
    DAQ::BO::CByteSwapper swapper(swapRequired);
    swapper.interpretAs(beg, size);
    swapper.interpretAs(beg+4, type);

}


template<class ByteIterator>
std::pair<CRingItemUPtr, ByteIterator>
parseLeaf(ByteIterator beg, ByteIterator end)
{
    if (std::distance(beg,end) < 8) {
        throw std::runtime_error("DAQ::V12::Parser::parseLeaf(ByteIterator,ByteIterator> insufficient data to parse.");
    }

    // check the type to see whether we need to swap
    bool swapRequired = mustSwap(beg+4, beg+8);

    Buffer::RangeDeserializer<ByteIterator> stream(beg, end, swapRequired);

    auto size = stream.template peek<std::uint32_t>();

    auto pItem = CRingItemFactory::createRingItem(beg, beg+size);

    return std::make_pair(std::move(pItem), beg+size);
}



template<class ByteIterator>
std::pair<CRingItemUPtr, ByteIterator>
parseCompound(ByteIterator beg, ByteIterator end)
{

    std::unique_ptr<CCompoundRingItem> pItem(new CCompoundRingItem);

    bool swapRequired = mustSwap(beg, beg+4);

    Buffer::RangeDeserializer<ByteIterator> stream(beg, end, swapRequired);

    uint32_t type, size, sourceId;
    uint64_t tstamp;

    size_t bytesInBuffer = std::distance(beg, end);
    if (bytesInBuffer < 20) {
        throw std::runtime_error("DAQ::V12::Parser::parseCompound() insufficient data to parse header");
    }

    stream >> size >> type >> tstamp >> sourceId;

    pItem->setType(type);
    pItem->setEventTimestamp(tstamp);
    pItem->setSourceId(sourceId);

    if (bytesInBuffer < size) {
        throw std::runtime_error("DAQ::V12::Parser::parseCompound() insufficient data to parse complete item");
    }

    // parse the body of the compound item
    auto it = stream.pos();
    while (it < end) {

        // We know that the first thing in the body of the compound item
        // will be a header and it will tell us when to use the

        bool swapNeeded;
        std::pair<CRingItemUPtr, ByteIterator> result;

        uint32_t size, type;
        peekHeaderType(it, end, size, type, swapNeeded);

        if (isCompound(type)) {
            result = parseCompound(it, std::min(it+size, end));
        } else {
            result = parseLeaf(it, std::min(it+size, end));
        }

        pItem->appendChild(std::move(result.first));
        it += size;

    }

    return std::make_pair(std::move(pItem), it);
}


/*! \brief Parse raw data
 *
 */
template<class ByteIterator> std::pair<CRingItemUPtr, ByteIterator>
parse(ByteIterator beg, ByteIterator end) {
    bool mustSwap;
    uint32_t size, type;

    peekHeaderType(beg, end, size, type, mustSwap);

    if (isCompound(type)) {
        return parseCompound(beg, std::min(beg+size, end));
    } else {
        return parseLeaf(beg, std::min(beg+size, end));
    }
}


} // end Parser
} // end V12
} // end DAQ
#endif // DAQ_V12_CRINGITEMPARSER_H
