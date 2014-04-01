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

#include <jsonrpc/rpc.h>
#include "Defaults.h" 
#include "Client.h" 
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

RPCServer::RPCServer() : AbstractServer<RPCServer>(new HttpServer(8080)){
  this->bindAndAddNotification(new Procedure("notifyServer", PARAMS_BY_NAME, NULL),
                               &RPCServer::notifyServer);
  
  this->bindAndAddMethod(new jsonrpc::Procedure("sayHello", PARAMS_BY_NAME, JSON_STRING, "name",
                                                JSON_STRING, NULL),
                         &RPCServer::sayHello);
}

void RPCServer::notifyServer(const Json::Value& request){
  cout << "Server got notified" << endl;
}
void RPCServer::sayHello(const Json::Value& request, Json::Value& response){
  response = "Hello " + request["name"].asString();
}

