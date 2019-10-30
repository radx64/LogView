#ifndef SERIALIZER_SERIALIZERLOGFILE_HPP
#define SERIALIZER_SERIALIZERLOGFILE_HPP

class Logfile;
class QJsonObject;

namespace serializer
{

class Logfile
{
public:
    Logfile() = delete;
    static void serialize(const ::Logfile &lf, QJsonObject &json);
    static void deserialize(::Logfile &lf, const QJsonObject &json);
};

}  // namespace serializer

#endif // SERIALIZER_SERIALIZERLOGFILE_HPP
