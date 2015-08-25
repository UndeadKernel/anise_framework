#ifndef LOGINFO_H
#define LOGINFO_H

#define tadasa



#include <QDateTime>
#include <QVariant>

class CLogInfo
{
public:

  enum class ESource {null, node, framework};
  #ifdef tadasa
  enum class EStatus {null,info,error,warning};
  #endif

  CLogInfo();

  // Convert this progress information to a string that we can show to the user.
  QString toJsonString();
  // Print the json representation of this log if is reporting
  // ... is enabled.
  void printMessage();


  ESource src() const;
  QString srcString() const;
  void setSrc(ESource src);

#ifdef tadasa
  EStatus status() const;
  QString statusString() const;
  void setStatus(EStatus state);
#endif

#ifdef tadasa
  QString msg() const;
  QString msgString() const;
  void setMsg(QString msg);
#endif

 // QVariant info() const;
 // void setInfo(QString info);
 // void setInfo(qint32 info);

  QString getSrc_name() const;
  void setName(QString src_name);

  QDateTime time() const;
  void setTime(QDateTime time);
//QString m_msg;
private:
  ESource m_src;
  EStatus m_status;
  QString src_name;
  QString m_msg;
  QDateTime m_time;

};


#endif // LOGINFO_H
