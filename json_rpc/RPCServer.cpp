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
/** @file RPCServer.cpp
 * @author Konstantin Kudryavtsev <konstantin@ethertools.net>
 * @date 2014
 * Ethereum client.
 */

#include "Defaults.h" 
#include "PeerNetwork.h" 
#include "BlockChain.h" 
#include "State.h" 
#include "FileSystem.h" 
#include "Instruction.h" 
#include "Common.h"
#include "RPCServer.h"

using namespace jsonrpc;
using namespace std;
using namespace eth;

#define ADD_QUOTES_HELPER(s) #s 
#define ADD_QUOTES(s) ADD_QUOTES_HELPER(s)

RPCServer::RPCServer():
  AbstractServer<RPCServer>(new HttpServer(8080)),
  client("Ethereum(++)/json_rpc", KeyPair::create().address())
{
  // this->bindAndAddNotification(new Procedure("notifyServer", PARAMS_BY_NAME, NULL),
  //                              &RPCServer::notifyServer);
  
  this->bindAndAddMethod(new Procedure("getLastBlock", PARAMS_BY_NAME, JSON_STRING, NULL),
                         &RPCServer::getLastBlock);

  client.startNetwork((short)30303);
  client.connect("54.201.28.117", (short)30303);
}

void RPCServer::getLastBlock(const Json::Value& req, Json::Value& res){
  //client.lock();
  auto const& bc = client.blockChain();
  auto h = bc.currentHash();
  auto b = bc.block();
  auto bi = BlockInfo(b);

  res["number"] = to_string(bc.details(bc.currentHash()).number);
  res["hash"] = boost::lexical_cast<string>(bi.hash);
  
  //client.unlock();
  
  //res = to_string(client->blockChain().details().number);
}


// void RPCServer::notifyServer(const Json::Value& request){
//   cout << "Server got notified" << endl;
// }
// void RPCServer::sayHello(const Json::Value& request, Json::Value& response){
//   response = "Hello " + request["name"].asString();
// }

