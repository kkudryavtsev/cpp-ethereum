/*
  This file is part of cpp-ethereum.

  cpp-ethereum is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  cpp-ethereum is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file main.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 * Ethereum client.
 */

#include <thread> 
#include <chrono> 
#include <fstream> 
#include <iostream> 
#include "Defaults.h" 
#include "Client.h" 
#include "PeerNetwork.h" 
#include "BlockChain.h" 
#include "State.h" 
#include "FileSystem.h" 
#include "Instruction.h" 
#include "Common.h"
#include "mongo/client/dbclient.h"
using namespace std;
using namespace eth;
using namespace mongo;

#define ADD_QUOTES_HELPER(s) #s 
#define ADD_QUOTES(s) ADD_QUOTES_HELPER(s)


int main(int argc, char** argv) {
  char* mongoUrl = getenv("ETHERTOOLS_MONGO_URL");
  char* mongoUser = getenv("ETHERTOOLS_MONGO_USER");
  char* mongoPass = getenv("ETHERTOOLS_MONGO_PASS");

  if(mongoUrl != NULL){
    cout << mongoUrl << endl;
  }else{
    cout << "Mongo URL envvar not found" << endl;
    return -2;
  }
  
  string errmsg;
  ConnectionString cs = ConnectionString::parse(mongoUrl, errmsg);
  
  if(!cs.isValid()){
    cout << "error parsing url: " << errmsg << endl;
    return EXIT_FAILURE;
  }
  

  boost::scoped_ptr<DBClientBase> mongoClient(cs.connect(errmsg));

  if (!mongoClient) {
    cout << "couldn't connect: " << errmsg << endl;
    return EXIT_FAILURE;
  }

  if(mongoUser != NULL && mongoPass != NULL){
    mongoClient->auth(BSON("user" << mongoUser <<
                           "userSource" << "ethertools" <<
                           "pwd" << mongoPass <<
                           "mechanism" << "MONGODB-CR"));
  }


  
  std::cout << "connected ok" << std::endl;

  string dbPath;
  string publicIP;
  string clientName;

  // Init defaults 
  Defaults::get();

  // Our address. 
  KeyPair us = KeyPair::create();
  Address coinbase = us.address();

  string configFile = getDataDir() + "/config.rlp";
  bytes b = contents(configFile);
  if (b.size()){
    RLP config(b);
    us = KeyPair(config[0].toHash<Secret>());
    coinbase = config[1].toHash<Address>();
  }else{
    RLPStream config(2);
    config << us.secret() << coinbase;
    writeFile(configFile, config.out());
  }

  if (!clientName.empty()) clientName += "/";
  
  Client c("Ethereum(++)/" + clientName + "v" ADD_QUOTES(ETH_VERSION) "/" ADD_QUOTES(ETH_BUILD_TYPE) "/" ADD_QUOTES(ETH_BUILD_PLATFORM), coinbase, dbPath);

  cout << "eth version " << ADD_QUOTES(ETH_VERSION) << endl;
  cout << "Build: " << ADD_QUOTES(ETH_BUILD_PLATFORM) << "/" << ADD_QUOTES(ETH_BUILD_TYPE) << endl;
  cout << "Starting network and connecting to peers..." << endl;
  g_logVerbosity = 1;

  c.startNetwork((short)30303);
  c.connect("54.201.28.117", (short)30303);
  int i = 0;
  unsigned int maxNumber = 0;

  while (true) {
    this_thread::sleep_for(chrono::milliseconds(10000));
    c.lock();
    cout << "Checking for changes..." << endl;
    cout << "Peers: " << c.peerCount() << endl;

    i++;

    unsigned int currentRunMax = 0;

    auto const& bc = c.blockChain();
    auto const& state = c.state();
    bool newBlockExported = false;

    for (auto h = bc.currentHash(); h != bc.genesisHash(); h = bc.details(h).parent) {
      auto b = bc.block(h);
      auto bi = BlockInfo(b);

      auto d = bc.details(h);

      if (d.number <= maxNumber) {
        cout << "Highest Block number in Chain: " << d.number << " is not higher than max imported number: " << maxNumber << endl;
        break;
      }
      newBlockExported = true;

      if (d.number > currentRunMax)
        currentRunMax = d.number;

      cout << "Block Number " << d.number << endl;

      auto_ptr<DBClientCursor> cursor = mongoClient
        ->query("ethertools.blocks", QUERY("hash" << boost::lexical_cast<string>(bi.hash)));

      if(cursor->itcount() > 0) continue; //block has been processed and saved
        
      BSONObj p = BSON(GENOID <<
                       "hash" << boost::lexical_cast<string>(bi.hash) <<
                       "number" << to_string(d.number) << 
                       "parentHash" << boost::lexical_cast<string>(bi.parentHash) <<
                       "sha3Uncles" << boost::lexical_cast<string>(bi.sha3Uncles) <<
                       "coinbaseAddress" << boost::lexical_cast<string>(bi.coinbaseAddress) <<
                       "stateRoot" << boost::lexical_cast<string>(bi.stateRoot) <<
                       "sha3Transactions" << boost::lexical_cast<string>(bi.sha3Transactions) <<
                       "difficulty" << boost::lexical_cast<string>(bi.difficulty) <<
                       "timestamp" << boost::lexical_cast<string>(bi.timestamp) <<
                       "nonce" << boost::lexical_cast<string>(bi.nonce));
          
      mongoClient->insert("ethertools.blocks", p);


      //TODO mining rewards don't look right:

      // Create virtual txs for the mining rewards
      auto _block = bc.block(h);
         
      // Addresses rewarded;
      // for (auto const& i : RLP(_block)[2])
      //   {
      //     BlockInfo uncle = BlockInfo::fromHeader(i.data());
      //     rewarded.push_back(uncle.coinbaseAddress);
      //   }
      // u256 m_blockReward = 1500 * finney;
      // u256 r = m_blockReward;
        
      // for (auto const& i : rewarded)
      //   {
      //     txFile << bi.hash << ";" << bi.hash << ";" << i << ";" << "Mining reward" << ";" << (m_blockReward * 7 / 8) << ";" << endl; 
      //     r += m_blockReward / 8;
      //   }
      // txFile << bi.hash << ";" << bi.hash << ";" << bi.coinbaseAddress << ";" << "Mining reward" << ";" << r << ";" << endl;

      for (auto const& i : RLP(_block)[1]) {
        Transaction t(i.data());

        string data = t.data.size() ? boost::lexical_cast<string>(eth::disassemble(t.data)) : "null";

        BSONObj p = BSON(GENOID <<
                         "sha3" << boost::lexical_cast<string>(t.sha3()) <<
                         "blockHash" << boost::lexical_cast<string>(bi.hash) <<
                         "receiveAddress" << boost::lexical_cast<string>(t.receiveAddress) <<
                         "safeSender" << boost::lexical_cast<string>(t.safeSender()) <<
                         "value" << boost::lexical_cast<string>(t.value) <<
                         "data" << data);
              
        mongoClient->insert("ethertools.transactions", p);
      }
    }

    if (newBlockExported) {
      // Export contract states
      auto acs = state.addresses();
      for (auto a : acs) {
        if (state.isContractAddress(a.first)) {
          auto mem = state.contractMemory(a.first);
          u256 next = 0;
          unsigned numerics = 0;
          bool unexpectedNumeric = false;
          stringstream s;
          for (auto ii : mem) {
            if (next < ii.first) {
              unsigned j;
              for (j = 0; j <= numerics && next + j < ii.first; ++j)
                s << (j < numerics || unexpectedNumeric ? " 0" : " STOP");
              unexpectedNumeric = false;
              numerics -= min(numerics, j);
              if (next + j < ii.first)
                s << " @" << showbase << hex << ii.first << " ";
            }
            else if (!next) {
              s << "@" << showbase << hex << ii.first << " ";
            }
            auto iit = c_instructionInfo.find((Instruction)(unsigned)ii.second);
            if (numerics || iit == c_instructionInfo.end() || (u256)(unsigned)iit->first != ii.second)// not an instruction or expecting an argument...
              {
                if (numerics)
                  numerics--;
                else
                  unexpectedNumeric = true;
                s << " " << showbase << hex << ii.second;
              }
            else
              {
                auto const& ii = iit->second;
                s << " " << ii.name;
                numerics = ii.additional;
              }
            next = ii.first + 1;
          }
            
          auto_ptr<DBClientCursor> cursor = mongoClient->
            query("ethertools.addresses",
                  QUERY("address" << boost::lexical_cast<string>(a.first) ));
          if(cursor->itcount() == 0) {
            BSONObj p = BSON(GENOID <<
                             "address" << boost::lexical_cast<string>(a.first) <<
                             "contract" << s.str());
            mongoClient->insert("ethertools.addresses", p);
          }
        } else {
          auto_ptr<DBClientCursor> cursor = mongoClient->
            query("ethertools.addresses",
                  QUERY("address" << boost::lexical_cast<string>(a.first)));
          
          if(cursor->itcount() == 0){
            BSONObj p = BSON(GENOID <<
                             "address" << boost::lexical_cast<string>(a.first) <<
                             "balance" << boost::lexical_cast<string>(a.second));
            mongoClient->insert("ethertools.addresses", p);
          }
        }
      } 
    }
    if (currentRunMax > maxNumber)
      maxNumber = currentRunMax;
    c.unlock();
  }


  return 0;
}
