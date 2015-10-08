#ifndef LOGINFO_H
#define LOGINFO_H

#include <QDateTime>
#include <QVariant>


class CLogInfo
{
  public:

    enum class ESource {null, node, framework};
    enum class EStatus {null, info, error, warning};

    CLogInfo();

    // Convert this progress information to a string that we can show to the user.
    QString toJsonString();
    // Print the json representation of this log if is reporting
    // ... is enabled.
    # define print() printMessage(__FILE__, __LINE__, Q_FUNC_INFO)
    void printMessage(const char *file = 0, int line = 0, const char *function = 0);

    ESource src() const;
    QString srcString() const;
    void setSrc(ESource src);

    EStatus status() const;
    QString statusString() const;
    void setStatus(EStatus state);

    QString msg() const;
    void setMsg(QString msg);

    QString getSrcName() const;
    void setName(QString m_src_name);

    QDateTime time() const;
    void setTime(QDateTime time);

  private:
    ESource m_src;
    EStatus m_status;
    QString m_src_name;
    QString m_msg;
    QDateTime m_time;
};


#endif // LOGINFO_H
