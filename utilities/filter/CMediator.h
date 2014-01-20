
#ifndef CMEDIATOR_H
#define CMEDIATOR_H

class CDataSource;
class CFilter;
class CDataSink;
class CRingItem;

class CMediator
{
  private:
    CDataSource* m_pSource; //!< the source
    CFilter* m_pFilter; //!< the filter 
    CDataSink* m_pSink; //!< the sink 

  public:
    // The constructor
    CMediator(CDataSource* source, CFilter* filter, CDataSink* sink);

    virtual ~CMediator();

  private:
    // Copy and assignment do not make sense because ownership
    // is not transferrable of the CDataSource and CDataSink.
    CMediator(const CMediator&);
    CMediator& operator=(const CMediator&);

  public:

    /**! Set the filter
      This transfers ownership of the object to this CMediator.
      Ownership of the previous filter is transfered to the caller.
  
      \param filter the new filter
      \return the old filter 
    */
    CFilter* setFilter( CFilter* filter) 
    {
        CFilter* old_filter = m_pFilter;
        m_pFilter = filter;
        return old_filter;
    }  

    /**! Set the source
      This transfers ownership of the object to this CMediator.
      Ownership of the previous source is transfered to the caller.
  
      \param source the new source
      \return the old source 
    */
    CDataSource* setDataSource( CDataSource* source) 
    {
        CDataSource* old_source = m_pSource;
        m_pSource = source;
        return old_source;
    }  
  
    /**! Set the sink
      This transfers ownership of the object to this CMediator.
      Ownership of the previous sink is transfered to the caller.
  
      \param sink the new sink
      \return the old sink 
    */
    CDataSink* setDataSink( CDataSink* sink) 
    {
        CDataSink* old_sink = m_pSink;
        m_pSink = sink;
        return old_sink;
    }  


    /**! Access to the filter 
    */
    CFilter* getFilter() { return m_pFilter;}


    /**! The main loop
    */
    void mainLoop();

  protected:
    /**! Delegate item to proper handler of filter
    */
    const CRingItem* handleItem(const CRingItem* item);

};

#endif
