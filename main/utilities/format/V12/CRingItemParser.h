/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
        Jeromy Tompkins
         NSCL
         Michigan State University
         East Lansing, MI 48824-1321
*/


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

/*!
 * \brief Checks whether bit 16 is set in the type value
 *
 * \param type  a type value (must be native byte order)
 *
 * \return boolean
 * \retval true    - bit 16 is set
 * \retval false   otherwise
 */
bool isComposite(uint32_t type);



/*! \brief Parses raw type and checks whether it indicates need to swap
 *
 *  \param beg   byte pointer to beginning of type field in raw data
 *  \param end   byte pointer to end of valid data for parsing
 *
 * \throws std::runtime_error if the range defined by [beg, end) is smaller than 2 bytes
 */
template<class ByteIterator>
bool mustSwap(ByteIterator beg, ByteIterator end) {

    if (std::distance(beg, end) < 2) {
        throw std::runtime_error("Parser::mustSwap() passed range with insufficient data.");
    }

    // we don't need to parse the full 4 bytes storing the type. Rather, we need
    // only to check whether the first two bytes are 0 or nonzero. If they are both
    // zero, that indicates the byte ordering is foreign to the local machine.
    return (*beg==0 && *(beg+1)==0);
}



/*! \brief Parse size and type
 *
 * As a consequency of parsing, we also determine the proper byte ordering.
 *
 * \param beg   byte pointer to beginning of size field in raw data packet
 * \param end   byte pointer to end of valid data for parsing
 * \param size  reference to integer that the extracted size will be stored in
 * \param type  reference to integer that the extracted type will be stored in
 * \param swapRequired reference to boolean that state of byte ordering will be stored
 *
 * \throws std::runtime_error if the range defined by [beg,end) is less than 8 bytes
 *
 */
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



/*! \brief Parses a leaf ring item from raw byte data
 *
 * \param   beg     byte pointer to beginning of a size field
 * \param   end     byte pointer to end of valid data for parsing
 *
 * \throws std::runtime_error if the range defined by [beg,end) is smaller than 8 bytes
 *
 * The user is encouraged to call Parser::parse() instead of this function.
 *
 * Ultimately this just calls the CRingItemFactory::createRingItem method. It sanely interprets
 * the byte data if it is swapped compared to native byte order.
 *
 * \returns a pair consisting of a unique_ptr to the newly created ring item and a pointer to the
 * next byte in the range past the ring item.
 *
 */
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



/*!
 *  \brief Parse a composite ring item from a range of raw byte data
 *
 * \param beg   byte pointer to the beginning of the byte data
 * \param end   byte pointer to the end of valid data for parsing
 *
 * \throws std::runtime_error if the range [beg,end) is smaller than 20 bytes
 *
 * The parsing function recursively calls itself until either an error has occurred
 * or the entire ring item has been parsed.
 *
 * The user is encouraged to call Parser::parse() instead of this function.
 *
 * \returns a pair. The first element contains the newly constructed ring item, and the second
 * item is a pointer(or iterator) to the next byte past the ring item that was parsed.
 *
 */
template<class ByteIterator>
std::pair<CRingItemUPtr, ByteIterator>
parseComposite(ByteIterator beg, ByteIterator end)
{

    std::unique_ptr<CCompositeRingItem> pItem(new CCompositeRingItem);

    bool swapRequired = mustSwap(beg, beg+4);

    Buffer::RangeDeserializer<ByteIterator> stream(beg, end, swapRequired);

    uint32_t type, size, sourceId;
    uint64_t tstamp;

    size_t bytesInBuffer = std::distance(beg, end);
    if (bytesInBuffer < 20) {
        throw std::runtime_error("DAQ::V12::Parser::parseComposite() insufficient data to parse header");
    }

    stream >> size >> type >> tstamp >> sourceId;

    pItem->setType(type);
    pItem->setEventTimestamp(tstamp);
    pItem->setSourceId(sourceId);

    if (bytesInBuffer < size) {
        throw std::runtime_error("DAQ::V12::Parser::parseComposite() insufficient data to parse complete item");
    }

    // parse the body of the Composite item
    auto it = stream.pos();
    while (it < end) {

        // We know that the first thing in the body of the Composite item
        // will be a header and it will tell us when to use the

        bool swapNeeded;
        std::pair<CRingItemUPtr, ByteIterator> result;

        uint32_t size, type;
        peekHeaderType(it, end, size, type, swapNeeded);

        if (isComposite(type)) {
            result = parseComposite(it, std::min(it+size, end));
        } else {
            result = parseLeaf(it, std::min(it+size, end));
        }

        pItem->appendChild(std::move(result.first));
        it += size;

    }

    return std::make_pair(std::move(pItem), it);
}


/*!
 * \brief Recurse the ring item tree structure and check for type consistency
 *
 * \param item  the ring item to recurse
 * \param type  the type to compare to
 *
 * \return boolean
 * \retval true if all nested ring items share types with the the same least significant 15 bits
 * \retval false otherwise
 */
bool isTypeConsistent(CRingItem& item, uint32_t type);


/*!
 * \brief Checks whether type1 and type2 are consistent
 *
 * \param type1
 * \param type2
 *
 * Consistency is when both type share the same least significant 15 bits. In other words,
 * COMP_PHYSICS_EVENT (i.e. 0x801e) and PHYSICS_EVENT (0x001e) are consistent.COMP_PHYSICS_EVENT
 * and PERIODIC_SCALERS (i.e. 0x0014) are not.
 *
 * \retval true if consistent
 * \retval false otherwise
 */
bool isTypeConsistent(uint32_t type1, uint32_t type2);



/*! \brief Extract a ring item from a range of raw byte data
 *
 * The parse function will extract the entire ring item beginning at beg. It will
 * not parse the entire range of data if the ring item pointed to by the beg pointer
 * is smaller than the range [beg, end). If the item to parse is a composite type,
 * the parser willrecursively parse the tree until all children have been properly
 * extracted.
 *
 * The user is intended to call this function over the parseLeaf and parseComposite
 * functions.
 *
 *
 * \param beg   byte pointer to the beginning of the ring item
 * \param end   byte pointer to the end of valid byte data for parsing
 *
 * This will throw if parseLeaf or parseComposite throw.
 *
 * \returns a pair. The first element of the pair is a pointer to the created ring item and the
 * second element is a pointer to the next byte past the ring item.
 */
template<class ByteIterator> std::pair<CRingItemUPtr, ByteIterator>
parse(ByteIterator beg, ByteIterator end) {
    bool mustSwap;
    uint32_t size, type;

    peekHeaderType(beg, end, size, type, mustSwap);

    std::pair<CRingItemUPtr, ByteIterator> result;
    if (isComposite(type)) {
        result = parseComposite(beg, std::min(beg+size, end));
    } else {
        result = parseLeaf(beg, std::min(beg+size, end));
    }

    if (! isTypeConsistent(*result.first, result.first->type()) ) {
        throw std::runtime_error("Parser::parse(ByteIterator, ByteIterator) parsed ring item does not have type consistency.");
    }

    return result;
}


} // end Parser
} // end V12
} // end DAQ
#endif // DAQ_V12_CRINGITEMPARSER_H
