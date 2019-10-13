#ifndef SERIALIZABLE_HPP
#define SERIALIZABLE_HPP

class QJsonObject;

class Serializable
{
public:
    virtual void serialize(QJsonObject &json) const = 0;
    virtual void deserialize(const QJsonObject &json) = 0;

    virtual ~Serializable() = default;
};

#endif // SERIALIZABLE_HPP
