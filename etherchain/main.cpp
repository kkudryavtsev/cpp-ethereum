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
using namespace std;
using namespace eth;

#define ADD_QUOTES_HELPER(s) #s 
#define ADD_QUOTES(s) ADD_QUOTES_HELPER(s) 

int main(int argc, char** argv)
{

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
  if (b.size())
    {
      RLP config(b);
      us = KeyPair(config[0].toHash<Secret>());
      coinbase = config[1].toHash<Address>();
    }
  else
    {
      RLPStream config(2);
      config << us.secret() << coinbase;
      writeFile(configFile, config.out());
    }

  if (!clientName.empty())
    clientName += "/";
  Client c("Ethereum(++)/" + clientName + "v" ADD_QUOTES(ETH_VERSION) "/" ADD_QUOTES(ETH_BUILD_TYPE) "/" ADD_QUOTES(ETH_BUILD_PLATFORM), coinbase, dbPath);

  cout << "eth version " << ADD_QUOTES(ETH_VERSION) << endl;
  cout << "Build: " << ADD_QUOTES(ETH_BUILD_PLATFORM) << "/" << ADD_QUOTES(ETH_BUILD_TYPE) << endl;
  cout << "Starting network and connecting to peers..." << endl;
  g_logVerbosity = 1;

  c.startNetwork((short)30303);
  c.connect("54.201.28.117", (short)30303);
  //c.connect("54.200.139.158", (short)30303);
  int i = 0;
  unsigned int maxNumber = 0;

  while (true) {
    this_thread::sleep_for(chrono::milliseconds(10000));
    c.lock();
    cout << "Checking for changes..." << endl;
    cout << "Peers: " << c.peerCount() << endl;

    //if (c.changed()) {
    //cout << "Change detected, exporting data..." << endl;
    i++;
    //string exportPath = "C:\\ethereum\\data\\" + to_string(i); 
    string exportPath = "/Users/megatv/ethereum_data/" + to_string(i) + "_";
    ofstream blockFile;
    blockFile.open(exportPath + "blocks.txt");
    ofstream txFile;
    txFile.open(exportPath + "tx.txt");
    ofstream addrFile;
    addrFile.open(exportPath + "addrDat.txt");

    unsigned int currentRunMax = 0;

    auto const& bc = c.blockChain();
    auto const& state = c.state();
    bool newBlockExported = false;

    for (auto h = bc.currentHash(); h != bc.genesisHash(); h = bc.details(h).parent)
      {
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
        // cout << "Hash " << bi.hash << endl; 
        // cout << "Parent Hash " << bi.parentHash << endl; 
        // cout << "Difficulty " << bi.difficulty << endl; 
        // cout << "Coinbase " << bi.coinbaseAddress << endl; 
        // cout << "Nonce " << bi.nonce << endl; 
        // cout << "SHA3(Tx) " << bi.sha3Transactions << endl; 
        // cout << "SHA3(Uncles) " << bi.sha3Uncles << endl; 
        // cout << "Root " << bi.stateRoot << endl; 
        // cout << "Time " << bi.timestamp << endl; 
        // cout << "Transactions: " << endl; 

        blockFile << bi.hash << ";" << d.number << ";" << bi.parentHash << ";" << bi.sha3Uncles << ";" << bi.coinbaseAddress << ";" << bi.stateRoot << ";" << bi.sha3Transactions << ";" << bi.difficulty << ";" << bi.timestamp << ";" << bi.nonce << endl;

        // Create virtual txs for the mining rewards
        auto _block = bc.block(h);
        Addresses rewarded;
        for (auto const& i : RLP(_block)[2]) //TODO da fuq is RLP?
          {
            BlockInfo uncle = BlockInfo::fromHeader(i.data());
            rewarded.push_back(uncle.coinbaseAddress);
          }
        u256 m_blockReward = 1500 * finney;
        u256 r = m_blockReward;

        for (auto const& i : rewarded)
          {
            txFile << bi.hash << ";" << bi.hash << ";" << i << ";" << "Mining reward" << ";" << (m_blockReward * 7 / 8) << ";" << endl; 
            r += m_blockReward / 8;
          }
        txFile << bi.hash << ";" << bi.hash << ";" << bi.coinbaseAddress << ";" << "Mining reward" << ";" << r << ";" << endl;

        for (auto const& i : RLP(_block)[1])
          {
            Transaction t(i.data());
            //cout << "Tx Hash " << t.sha3() << endl;
            //cout << "Sender " << t.safeSender() << endl;
            //cout << "Recipient " << t.receiveAddress << endl;
            //cout << "Value " << t.value << endl;

            txFile << t.sha3() << ";" << bi.hash << ";" << t.receiveAddress << ";" << t.safeSender() << ";" << t.value;

            if (t.receiveAddress)
              {
                txFile << ";" << "0";
              }
            else
              {
                txFile << ";" << "1";
              }

            if (t.data.size())
              {
                txFile << ";" << eth::disassemble(t.data) << endl;
              }
            else 
              {
                txFile << ";" << endl;
              }
          }
      }

    if (newBlockExported)
      {
        // Export contract states
        auto acs = state.addresses();
        for (auto a : acs)
          {
            if (state.isContractAddress(a.first))
              {
                auto mem = state.contractMemory(a.first);
                u256 next = 0;
                unsigned numerics = 0;
                bool unexpectedNumeric = false;
                stringstream s;
                for (auto ii : mem)
                  {
                    if (next < ii.first)
                      {
                        unsigned j;
                        for (j = 0; j <= numerics && next + j < ii.first; ++j)
                          s << (j < numerics || unexpectedNumeric ? " 0" : " STOP");
                        unexpectedNumeric = false;
                        numerics -= min(numerics, j);
                        if (next + j < ii.first)
                          s << " @" << showbase << hex << ii.first << " ";
                      }
                    else if (!next)
                      {
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
                addrFile << a.first << ";" << s.str() << endl;
              }
          }
      }

    blockFile.close();
    txFile.close();
    addrFile.close();

    if (currentRunMax > maxNumber)
      maxNumber = currentRunMax;
    c.unlock();
    //}
  }
  return 0;
}
