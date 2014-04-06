/*
    This file is part of cpp-ethereum.

    cpp-ethereum is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    cpp-ethereum is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.    See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with cpp-ethereum.    If not, see <http://www.gnu.org/licenses/>.
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
	AbstractServer<RPCServer>(new HttpServer(4000)),
	m_client("Ethereum(++)/json_rpc", KeyPair::create().address())
{
	this->bindAndAddMethod(new Procedure("getLastBlock", PARAMS_BY_NAME, JSON_STRING, NULL),
						   &RPCServer::getLastBlock);
	g_logVerbosity = 1;
	m_client.startNetwork((short)30303);
	m_client.connect("54.72.31.55", (short)30303);
}

void RPCServer::getLastBlock(const Json::Value& req, Json::Value& res)
{
	m_client.lock();
	auto const& bc = m_client.blockChain();
	auto h = bc.currentHash();
	auto b = bc.block();
	auto bi = BlockInfo(b);
	res["number"] = to_string(bc.details(bc.currentHash()).number);
	res["hash"] = boost::lexical_cast<string>(bi.hash);
	res["parentHash"] = boost::lexical_cast<string>(bi.parentHash);
	res["sha3Uncles"] = boost::lexical_cast<string>(bi.sha3Uncles);
	res["coinbaseAddress"] = boost::lexical_cast<string>(bi.coinbaseAddress);
	res["stateRoot"] = boost::lexical_cast<string>(bi.stateRoot);
	res["sha3Transactions"] = boost::lexical_cast<string>(bi.sha3Transactions);
	res["difficulty"] = boost::lexical_cast<string>(bi.difficulty);
	res["timestamp"] = boost::lexical_cast<string>(bi.timestamp);
	res["nonce"] = boost::lexical_cast<string>(bi.nonce);
	res["transactions"] = getTransactions(b);
	m_client.unlock();
}


Json::Value RPCServer::getTransactions(bytesConstRef block)
{
	Json::Value txs_j;
	int i = 0;
	for (auto const& bRlp : RLP(block)[1])
	{
		Transaction t(bRlp.data());
		Json::Value tx_j;
		string data = t.data.size() ? boost::lexical_cast<string>(eth::disassemble(t.data)) : "null";
		string sha3 = boost::lexical_cast<string>(t.sha3());
		tx_j["sha3"] = sha3;
		tx_j["receiveAddress"] = boost::lexical_cast<string>(t.receiveAddress);
		tx_j["safeSender"] = boost::lexical_cast<string>(t.safeSender());
		tx_j["value"] = boost::lexical_cast<string>(t.value);
		tx_j["data"] = data;
		txs_j[i++].append(tx_j);
	}
	return txs_j;
}
