#include <stdexcept>
#include <string>
#include "OnlineDB/Oracle/interface/Oracle.h"
#include <boost/lexical_cast.hpp>

#include "OnlineDB/EcalCondDB/interface/RunTTErrorsDat.h"
#include "OnlineDB/EcalCondDB/interface/RunIOV.h"

using namespace std;
using namespace oracle::occi;

RunTTErrorsDat::RunTTErrorsDat()
{
  m_env = nullptr;
  m_conn = nullptr;
  m_writeStmt = nullptr;
  m_readStmt = nullptr;
 
  m_errorBits = 0;
}



RunTTErrorsDat::~RunTTErrorsDat()
{
}



void RunTTErrorsDat::prepareWrite()
  noexcept(false)
{
  this->checkConnection();

  try {
    m_writeStmt = m_conn->createStatement();
    /* Using TO_NUMBER because OCCI does not support 64-bit integers well */
    m_writeStmt->setSQL("INSERT INTO run_tt_errors_dat (iov_id, logic_id, "
			"error_bits) "
			"VALUES (:iov_id, :logic_id, "
			"to_number(:error_bits))");
  } catch (SQLException &e) {
    throw(std::runtime_error(std::string("RunTTErrorsDat::prepareWrite():  ")+getOraMessage(&e)));
  }
}



void RunTTErrorsDat::writeDB(const EcalLogicID* ecid, const RunTTErrorsDat* item, RunIOV* iov)
  noexcept(false)
{
  this->checkConnection();
  this->checkPrepare();

  int iovID = iov->fetchID();
  if (!iovID) { throw(std::runtime_error("RunTTErrorsDat::writeDB:  IOV not in DB")); }

  int logicID = ecid->getLogicID();
  if (!logicID) { throw(std::runtime_error("RunTTErrorsDat::writeDB:  Bad EcalLogicID")); }
  
  try {
    m_writeStmt->setInt(1, iovID);
    m_writeStmt->setInt(2, logicID);
    m_writeStmt->setString(3, std::to_string(item->getErrorBits()));
    m_writeStmt->executeUpdate();
  } catch (SQLException &e) {
    throw(std::runtime_error(std::string("RunTTErrorsDat::writeDB():  ")+getOraMessage(&e)));
  }
}



void RunTTErrorsDat::fetchData(map< EcalLogicID, RunTTErrorsDat >* fillMap, RunIOV* iov)
  noexcept(false)
{
  this->checkConnection();
  fillMap->clear();

  iov->setConnection(m_env, m_conn);
  int iovID = iov->fetchID();
  if (!iovID) { 
    //  throw(std::runtime_error("RunTTErrorsDat::writeDB:  IOV not in DB")); 
    return;
  }

  try {

    /* Using TO_CHAR because OCCI does not support 64-bit integers well */
    m_readStmt->setSQL("SELECT cv.name, cv.logic_id, cv.id1, cv.id2, cv.id3, cv.maps_to, "
		 "to_char(d.error_bits) "
		 "FROM channelview cv JOIN run_tt_errors_dat d "
		 "ON cv.logic_id = d.logic_id AND cv.name = cv.maps_to "
		 "WHERE d.iov_id = :iov_id");
    m_readStmt->setInt(1, iovID);
    ResultSet* rset = m_readStmt->executeQuery();
    
    std::pair< EcalLogicID, RunTTErrorsDat > p;
    RunTTErrorsDat dat;
    while(rset->next()) {
      p.first = EcalLogicID( getOraString(rset,1),     // name
			     rset->getInt(2),        // logic_id
			     rset->getInt(3),        // id1
			     rset->getInt(4),        // id2
			     rset->getInt(5),        // id3
			     getOraString(rset,6));    // maps_to

      dat.setErrorBits( boost::lexical_cast<uint64_t>(getOraString(rset,7)) );

      p.second = dat;
      fillMap->insert(p);
    }

  } catch (SQLException &e) {
    throw(std::runtime_error(std::string("RunTTErrorsDat::fetchData():  ")+getOraMessage(&e)));
  }
}
