#include "rediscpp.h"
#ifdef _WIN32
#include <winsock.h>
#endif // _WIN32

#include <sstream>

RedisClient::RedisClient()
{
	m_pCtx = NULL;
}

RedisClient::~RedisClient()
{
	if (m_pCtx)
	{
		redisFree(m_pCtx);
	}
}

bool RedisClient::Connect(const char *ip, int port, int timeoutsec)
{
	struct timeval tv;
	tv.tv_sec = timeoutsec;
	tv.tv_usec = 0;

	m_pCtx = redisConnectWithTimeout(ip, port, tv);
	if (m_pCtx == NULL)
	{
		return false;
	}

	if (m_pCtx->err)
	{
		redisFree(m_pCtx);
		return false;
	}

	return true;
}

bool RedisClient::Connect(const char *ip, int port, int db, const char *password, int timeoutsec)
{
	bool ret = this->Connect(ip, port, timeoutsec);
	if (!ret)
	{
		return false;
	}

	if (password != NULL)
	{
		RedisReplyObject reply ((redisReply *)redisCommand(m_pCtx, "AUTH %s", password));

		if (reply == NULL)
		{
			return false;
		}

		if (strcmp(reply->str, "OK") != 0)
		{
			return false;
		}

		RedisReplyObject reply2((redisReply *)redisCommand(m_pCtx, "SELECT %d", db));
		if (reply2 == NULL)
		{
			return false;
		}

		if (strcmp(reply2->str, "OK") != 0)
		{
			return false;
		}

	}

	return true;

}


string RedisClient::Get(RedisKey key)
{
	RedisReplyObject reply((redisReply *)redisCommand(m_pCtx, "GET %s", key));
	if (reply == NULL)
	{
		return NULL;
	}

	string s(reply->str);
	
	return s;
}

bool RedisClient::Set(RedisKey key, const char *value)
{
	RedisReplyObject reply((redisReply *)redisCommand(m_pCtx, "SET %s %s",key, value));

	if (reply == NULL)
	{
		return false;
	}

	if (reply->type == REDIS_REPLY_STATUS && strcmp(reply->str, "OK") == 0)
	{
		return true;
	}

	return false;
}

bool RedisClient::Del(RedisKey key)
{
	RedisReplyObject reply((redisReply *)redisCommand(m_pCtx, "DEL %s", key));

	if (reply == NULL)
	{
		return false;
	}

	if (reply->type == REDIS_REPLY_INTEGER && reply->integer == 1)
	{
		return true;
	}

	return false;
}

bool RedisClient::Expire(RedisKey key, int seconds)
{
	RedisReplyObject reply((redisReply *)redisCommand(m_pCtx, "EXPIRE %s %d", key, seconds));
	if (reply == NULL)
	{
		return false;
	}

	if (reply->type == REDIS_REPLY_INTEGER && reply->integer == 1)
	{
		return true;
	}

	return false;
}

bool RedisClient::ExpireAt(RedisKey key, time_t timestamp)
{
	RedisReplyObject reply((redisReply *)redisCommand(m_pCtx, "EXPIREAT %s %lld", key, timestamp));
	if (reply == NULL)
	{
		return false;
	}

	if (reply->type == REDIS_REPLY_INTEGER && reply->integer == 1)
	{
		return true;
	}

	return false;
}

bool RedisClient::Persist(RedisKey key)
{
	RedisReplyObject reply((redisReply*)redisCommand(m_pCtx, "PERSIST %s", key));
	if (reply == NULL)
	{
		return false;
	}

	if (reply->type == REDIS_REPLY_INTEGER && reply->integer == 1)
	{
		return true;
	}

	return false;
}

bool RedisClient::PExpire(RedisKey key, int milliseconds)
{
	RedisReplyObject reply((redisReply*)redisCommand(m_pCtx, "PEXPIRE %s %d", key, milliseconds));
	if (reply == NULL)
	{
		return false;
	}

	if (reply->type == REDIS_REPLY_INTEGER && reply->integer == 1)
	{
		return true;
	}

	return false;
}

bool RedisClient::PExpireAt(RedisKey key, time_t  timestamp_ms)
{
	RedisReplyObject reply((redisReply*)redisCommand(m_pCtx, "PEXPIREAT %s %lld", key, timestamp_ms));
	if (reply == NULL)
	{
		return false;
	}

	if (reply->type == REDIS_REPLY_INTEGER && reply->integer == 1)
	{
		return true;
	}

	return false;
}

string RedisClient::HGet(RedisKey key, const char *field)
{
	RedisReplyObject reply((redisReply *)redisCommand(m_pCtx, "HGET %s %s", key, field));
	if (reply == NULL)
	{
		return NULL;
	}

	string s(reply->str);

	return s;
}

RedisMapType RedisClient::HGetall(RedisKey key)
{
	RedisMapType retMap;
	RedisReplyObject reply((redisReply *)redisCommand(m_pCtx, "HGETALL %s", key));
	if (reply == NULL)
	{
		return retMap;
	}

	if (reply->type != REDIS_REPLY_ARRAY || reply->elements <= 0 || reply->elements%2 != 0)
	{
		return retMap;
	}

	for (int i = 0; i < (int)reply->elements;)
	{
		redisReply *keyElem = reply->element[i];
		redisReply *valueElem = reply->element[i + 1];
		if (keyElem->type == REDIS_REPLY_STRING && valueElem->type == REDIS_REPLY_STRING)
		{
			retMap.insert(RedisMapType::value_type(keyElem->str, valueElem->str));
		}
		i += 2;
	}

	return retMap;
}

bool RedisClient::HDel(RedisKey key, RedisField field)
{
	RedisReplyObject reply((redisReply *)redisCommand(m_pCtx, "HDEL %s %s", key, field));

	if (reply == NULL)
	{
		return false;
	}

	if (reply->type == REDIS_REPLY_INTEGER && reply->integer == 1)
	{
		return true;
	}

	return false;
}

int RedisClient::HIncrby(RedisKey key, RedisField field, PORT_LONGLONG value)
{
	RedisReplyObject reply((redisReply *)redisCommand(m_pCtx, "HINCRBY %s %s %lld", key, field, value));
	if (reply == NULL)
	{
		return 0;
	}

	if (reply->type != REDIS_REPLY_INTEGER)
	{
		return 0;
	}

	return reply->integer;
}

RedisArrayType RedisClient::LRange(RedisKey key, int beg, int end)
{
	RedisArrayType retResult;
	RedisReplyObject reply((redisReply *)redisCommand(m_pCtx, "LRANGE %s %d %d", key, beg, end));
	if (reply == NULL)
	{
		return retResult;
	}

	if (reply->type != REDIS_REPLY_ARRAY || reply->elements <= 0)
	{
		return retResult;
	}

	for (int i = 0; i < (int)reply->elements; ++i)
	{
		redisReply *elem = reply->element[i];
		if (elem->type == REDIS_REPLY_STRING)
		{
			retResult.push_back(RedisArrayType::value_type(elem->str));
		}
	}


	return retResult;
}


RedisArrayType RedisClient::LRangeall(RedisKey key)
{
	return LRange(key, 0, -1);
}

bool RedisClient::Exists(RedisKey key)
{
	RedisReplyObject reply((redisReply *)redisCommand(m_pCtx, "EXISTS %s", key));
	if (reply == NULL)
	{
		return false;
	}

	if (reply->type != REDIS_REPLY_INTEGER)
	{
		return false;
	}

	return reply->integer == 1;
}

bool RedisClient::HExists(RedisKey key, RedisField field)
{
	RedisReplyObject reply((redisReply *)redisCommand(m_pCtx, "HEXISTS %s %s", key, field));
	if (reply == NULL)
	{
		return false;
	}

	if (reply->type != REDIS_REPLY_INTEGER)
	{
		return false;
	}

	bool ret = reply->integer == 1;


	return ret;
}

int RedisClient::HLen(RedisKey key)
{
	RedisReplyObject reply((redisReply *)redisCommand(m_pCtx, "HLEN %s", key));
	if (reply == NULL)
	{
		return -1;
	}

	if (reply->type != REDIS_REPLY_INTEGER)
	{
		return -1;
	}

	return reply->integer;
}

RedisArrayType RedisClient::HKeys(RedisKey key)
{
	RedisArrayType retResult;
	RedisReplyObject reply((redisReply *)redisCommand(m_pCtx, "HKEYS %s", key));
	if (reply == NULL)
	{
		return retResult;
	}

	if (reply->type != REDIS_REPLY_ARRAY || reply->elements <= 0)
	{
		return retResult;
	}

	for (int i = 0; i < (int)reply->elements; ++i)
	{
		redisReply *elem = reply->element[i];
		if (elem->type == REDIS_REPLY_STRING)
		{
			retResult.push_back(RedisArrayType::value_type(elem->str));
		}
	}


	return retResult;
}

RedisArrayType RedisClient::MGet(RedisKey keys)
{
	stringstream os;
	os << "MGET " << keys;
	RedisArrayType retResult;
	RedisReplyObject reply((redisReply *)redisCommand(m_pCtx, os.str().c_str()));
	if (reply == NULL)
	{
		return retResult;
	}

	if (reply->type != REDIS_REPLY_ARRAY || reply->elements <= 0)
	{
		return retResult;
	}

	for (int i = 0; i < (int)reply->elements; ++i)
	{
		redisReply *elem = reply->element[i];
		if (elem->type == REDIS_REPLY_STRING)
		{
			retResult.push_back(RedisArrayType::value_type(elem->str));
		}
	}

	return retResult;

}

bool RedisClient::CheckReplyStatus(const redisReply *reply)
{
	if (reply == NULL)
	{
		return false;
	}

	if (reply->type == REDIS_REPLY_STATUS && strcmp(reply->str, "OK") == 0)
	{
		return true;
	}

	return false;
}
