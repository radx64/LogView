#ifndef SERIALIZER_SERIALIZEGREP_NODE_HPP
#define SERIALIZER_SERIALIZEGREP_NODE_HPP

#include <memory>
#include <string>
#include <vector>

class GrepNode;
class QJsonObject;

namespace serializer
{

class GrepNode
{
public:
GrepNode() = delete;

static void serialize(const ::GrepNode &gp, QJsonObject &json);
static void deserialize(::GrepNode &gp, const QJsonObject &json);
};

}  // namespace serializer
#endif // SERIALIZER_SERIALIZEGREP_NODE_HPP
