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

#ifndef DAQ_V12_CRINGITEM_H
#define DAQ_V12_CRINGITEM_H

#include <cstdint>
#include <string>
#include <memory>
#include <vector>

namespace DAQ {
    namespace V12 {

    // Some type defs and forward declarations
    class CRingItem;
    using CRingItemPtr = std::shared_ptr<CRingItem>;
    using CRingItemUPtr = std::unique_ptr<CRingItem>;

    class CRawRingItem;

    /*!
     * \brief The CRingItem class
     *
     * The CRingItem class is a pure virtual class. It holds absolutely no
     * data and does not implement any method. Rather it just defines the
     * interface that all ring items must implement.
     *
     * The CRingItem class is intended to represent
     * the version 12 representation of NSCL data. The data format supports
     * ring items that can be nested and laid out as a tree structure. In that
     * regard, we often refer to leaf and composite types. The derived classes
     * of this base class implement those leaf and composite types. There is only
     * one derived type that is intended to represent the composite items in a
     * high level way. The CCompositeRingItem serves this purpose. The CRawRingItem
     * is another derived class that can represent a composite ring item, but it
     * stores all of the children as byte data. All other ring items are leaf types.
     *
     */
    class CRingItem {
      public:

        /*!
         * \return the full type of the item (including the composite bit)
         */
        virtual uint32_t type() const = 0;
        virtual void setType(uint32_t type) = 0;

        /*!
         * The size returned by this method should return the number of
         * bytes that would represent the serialized form of the ring item.
         * It must not return, for example, sizeof(DerivedRingItem).
         *
         * \return the size of the ring item in bytes
         */
        virtual uint32_t size() const = 0;

        virtual uint64_t getEventTimestamp() const = 0;
        virtual void setEventTimestamp(uint64_t tstamp) = 0;

        virtual uint32_t getSourceId() const = 0;
        virtual void setSourceId(uint32_t id) = 0;

        /*!
         * \brief Whether or not the body of the ring item consists of nested ring items
         *
         * Every derived type besides the CCompositRingItem and CRawRingItem
         * classes will be leaf types.
         *
         * \return true if ring item is a composite type
         * \return false if ring item is a leaf type
         */
        virtual bool     isComposite() const = 0;

        /*!
         * \return whether the body data returned to the user is not in native byte order
         */
        virtual bool     mustSwap() const = 0;

        /*!
         * \return a name for the type of item
         */
        virtual std::string typeName() const = 0;

        /*!
         * \return a string representation of the ring item
         */
        virtual std::string toString() const = 0;

        /*!
         * \brief Serialize the ring item to a raw ring item
         *
         * Implementations of this method should take care to clear the
         * body of the raw ring item before filling it with data.
         *
         * \param item  a raw ring item to fill with data
         */
        virtual void toRawRingItem(CRawRingItem& item) const = 0;


        /*!
         * \brief Equality comparison operator
         *
         * \param item the item to compare this to
         *
         *
         * \retval true if item and have this have identical state
         * \retval false otherwise
         */
        virtual bool operator==(const CRingItem& item) const = 0;

        /*!
         * \brief Inequality comparison operator
         *
         * \param item the item to compare to
         *
         * \retval true if equality condition is not true
         * \retval false otherwise
         */
        virtual bool operator!=(const CRingItem& item) const = 0;


    };

    /*!
     * \param item  the item whose header will be represented
     *
     * \return a string representation of the header data
     */
    std::string headerToString(const CRingItem& item);

} // end of V12 namespace
} // end DAQ

#endif
