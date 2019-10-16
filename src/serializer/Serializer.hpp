#ifndef SERIALIZABLE_HPP
#define SERIALIZABLE_HPP

class QJsonObject;

namespace serializer
{

class Serializer
{
public:
    virtual void serialize(QJsonObject &json) const = 0;
    virtual void deserialize(const QJsonObject &json) = 0;
    virtual ~Serializer() = default;
};

}  // namespace serialzer

#endif // SERIALIZABLE_HPP
