#include "entropyfeaturesnode.h"
#include "data/datafactory.h"
#include "data/messagedata.h"
#include "tabledata/tabledata.h"
#include <QDebug>
#include <QList>
#include <QHash>
#include <QtMath>
#include <algorithm>


//------------------------------------------------------------------------------
// Constructor and Destructor

CEntropyfeaturesNode::CEntropyfeaturesNode(const CNodeConfig &config, QObject *parent/* = 0*/)
    : CNode(config, parent)
{

}


//------------------------------------------------------------------------------
// Public Functions

void CEntropyfeaturesNode::configure(CNodeConfig &config)
{
    // Set a Description of this node.
    config.setDescription("Calculate the entropy of a given table attribute.");

    // Add parameters
    config.addBool("use_start_time", "Use Flows's Starting Time",
                   "Should the start time of a flow be used to place the flows into windows of time?", true);
    config.addUInt("time_start_attr", " Start Time Index",
        "The index of the attribute that holds the starting timestamp of the received table.", 0);
    config.addUInt("time_end_attr", "End Time Index",
        "The index of the attribute that holds the end timestamp of the received table.", 1);
    config.addUInt("feature_attr", "Feature Index",
        "The index of the attribute we wish to use to calculate the entropy", 2);
    config.addBool("time_milli", "Time in Milliseconds",
        "Time is indicated in milleseconds after the Unix Epoch", true);
    config.addUInt("time_window", "Window of Time", "Time in seconds for a window of time.", 60);
    config.addBool("normalize_entropy","Normalize Entropy",
        "Normalize the entropy for each time window between 0 and 100.", false);


    // Add the gates.
    config.addInput("in", "table");
    config.addOutput("out", "table");
}


//------------------------------------------------------------------------------
// Protected Functions

bool CEntropyfeaturesNode::start()
{
    return true;
}

bool CEntropyfeaturesNode::data(QString gate_name, const CConstDataPointer &data)
{
    Q_UNUSED(gate_name);

    if(data->getType() != "table") {
        // Do not process anything that is not a table.
        return false;
    }

    // Get the user parameters
    bool use_start_time = getConfig().getParameter("use_start_time")->value.toBool();
    uint time_start_attr = getConfig().getParameter("time_start_attr")->value.toUInt();
    uint time_end_attr = getConfig().getParameter("time_end_attr")->value.toUInt();
    uint feature_attr = getConfig().getParameter("feature_attr")->value.toUInt();
    bool time_milli = getConfig().getParameter("time_milli")->value.toBool();
    uint time_window = getConfig().getParameter("time_window")->value.toUInt();
    bool normalize_entropy = getConfig().getParameter("normalize_entropy")->value.toBool();

    QSharedPointer<const CTableData> flows_table = data.staticCast<const CTableData>();
    QSharedPointer<CTableData> entropy_table = autoCreateData<CTableData>("table");

    quint32 total_flows = flows_table->rowCount();
    if(total_flows == 0) {
        // Nothing to analyze, send empty table and exit.
        commit("out", entropy_table);
        return true;
    }

    // The time can be in seconds or milliseconds, account for that.
    quint64 time_multiplier = 1;
    if(time_milli) {
        time_multiplier = 1000;
    }

    // Are we taking into account the time windows starting from when the first flow
    // ... started or when the first flow finished?
    quint64 start_time;
    if(use_start_time) {
        // Use the time when a flow started
        // The start time is always in the first record
        start_time = flows_table->getRow(0).at(time_start_attr).toULongLong();
    } else {
        // Use only the time when a flow finished.
        // We need to search for the last end time as these are not sorted.
        start_time = findFirstEndTime(flows_table, time_start_attr, time_end_attr);
    }
    // The end time has to be found within all records
    quint64 end_time = findLastEndTime(flows_table, time_end_attr);
    quint32 total_windows = qCeil((double)(end_time - start_time)
        / (double)(time_window * time_multiplier));

    // One list of counts for each time window.
    QList<QHash<QVariant, quint32> > time_windows;
    for(quint32 i = 0; i < total_windows; ++i) {
        // Initialize the time window's structure that will hold the counts.
        time_windows.append(QHash<QVariant, quint32>());
    }

    // Iterate each flow and add the chosen feature into the counts of the corresponding windows.
    for(quint32 i = 0; i < total_flows; ++i) {
        const QList<QVariant> &flow = flows_table->getRow(i);

        // Calculate the time window of the end of the flow.
        quint64 end_flow_time = flow.at(time_end_attr).toULongLong();
        quint32 end_flow_window = qCeil((double)(end_flow_time - start_time)
            / (double)(time_window * time_multiplier));
        if(end_flow_window != 0) {
            --end_flow_window;
        }

        // Calculate the time window of the start of the flow
        quint32 start_flow_window;
        if(use_start_time) {
            // Calculate the window of time
            quint64 start_flow_time = flow.at(time_start_attr).toULongLong();
            start_flow_window = qCeil((double)(start_flow_time - start_time)
                / (double)(time_window * time_multiplier));
            if(start_flow_window != 0) {
                --start_flow_window;
            }
        } else {
            // We are not using the start time, use the same window as the last one.
            start_flow_window = end_flow_window;
        }

        // For each window of time this flow was in, update its counts.
        for(quint32 j = start_flow_window; j <= end_flow_window; ++j) {
            QHash<QVariant, quint32> &window = time_windows[j];
            const QVariant &key = flow.at(feature_attr);
            quint32 count = window.value(key, 0);
            window.insert(key, count + 1);
        }
    }

    // Calculate the entropy of each time window.
    QList<QVariant> &entropy_row = entropy_table->newRow();
    entropy_row.reserve(total_windows);
    QList<QHash<QVariant, quint32>>::iterator i;
    for(i = time_windows.begin(); i != time_windows.end(); ++i) {
        QList<quint32> counts = i->values();
        QList<quint32>::iterator j;

        // Efficient calculation of entropy using H() = logS - 1/S (SUM_i ni log(ni))

        // Calculate the normalization constant.
        double entropy = 0;
        double S = 0;
        double unnorm = 0;
        for(j = counts.begin(); j != counts.end(); ++j) {
            double ni = static_cast<double>(*j);
            if(ni == 0) {
                continue;
            }
            unnorm += ni * std::log2(ni);
            S += ni;
        }

        if(S > 0) {
            entropy = std::log2(S) - (1.0 / S) * unnorm;
            if(normalize_entropy) {
                double max_entropy = std::log2(counts.size());
                if(max_entropy != 0.0) {
                    // Normalize the entropy between 0 and 100
                    entropy = entropy * (100.0 / max_entropy);
                }
            }
        }

        entropy_row.append(entropy);

//   Inefficient Entropy calculation:
//        double norm = 0;
//        for(j = counts.begin(); j != counts.end(); ++j) {
//            norm += *j;
//        }
//        // Calcuate the entropy value.
//        double entropy = 0;
//        for(j = counts.begin(); j != counts.end(); ++j) {
//            double n = static_cast<double>(*j) / norm;
//            entropy -=  n * std::log2(n);
//        }
//        entropy_row.append(entropy);
    }

    commit("out", entropy_table);

    // Return true as we processed the data.
    return true;
}

quint64 CEntropyfeaturesNode::findLastEndTime(
    const QSharedPointer<const CTableData> &flows_table,
    quint32 time_end_attr)
{
    quint32 total_flows = flows_table->rowCount();

    // Assume the biggest time is in the first attribute and then
    // ... iterate the array inversely up to index 1.
    quint64 end_time = flows_table->getRow(0).at(time_end_attr).toULongLong();
    quint64 current_time = 0;
    for(quint32 i = total_flows - 1; i > 0; --i) {
        current_time = flows_table->getRow(i).at(time_end_attr).toULongLong();
        if(current_time > end_time) {
            end_time = current_time;
        }
    }

    return end_time;
}

quint64 CEntropyfeaturesNode::findFirstEndTime(
    const QSharedPointer <const CTableData> &flows_table,
    quint32 time_start_attr,
    quint32 time_end_attr)
{
    quint32 total_flows = flows_table->rowCount();

    // Assume the biggest time is in the first attribute and then
    // ... iterate the array inversely up to index 1.
    quint64 end_time = flows_table->getRow(0).at(time_end_attr).toULongLong();
    quint64 current_time = 0;
    for(quint32 i = 1; i < total_flows; ++i) {
        current_time = flows_table->getRow(i).at(time_end_attr).toULongLong();
        if(current_time < end_time) {
            end_time = current_time;
        }

        // Stop comparing if the starting time is bigger than the end time.
        // ... The starting time is assumed to always be ordered.
        if(end_time < flows_table->getRow(i).at(time_start_attr).toULongLong()) {
            return end_time;
        }
    }

    return end_time;
}

uint qHash(const QVariant &var)
{
    if (!var.isValid() || var.isNull()) {
        return -1;
    }

    switch (var.type()) {
      case QVariant::Int:
        return qHash(var.toInt());
        break;
      case QVariant::UInt:
        return qHash(var.toUInt());
        break;
      case QVariant::Bool:
        return qHash(var.toUInt());
        break;
      case QVariant::Double:
        return qHash(var.toUInt());
        break;
      case QVariant::LongLong:
        return qHash(var.toLongLong());
        break;
      case QVariant::ULongLong:
        return qHash(var.toULongLong());
        break;
      case QVariant::String:
        return qHash(var.toString());
        break;
      case QVariant::Char:
        return qHash(var.toChar());
        break;
      case QVariant::StringList:
        return qHash(var.toString());
        break;
      case QVariant::ByteArray:
        return qHash(var.toByteArray());
        break;
      case QVariant::Date:
      case QVariant::Time:
      case QVariant::DateTime:
      case QVariant::Url:
      case QVariant::Locale:
      case QVariant::RegExp:
        return qHash(var.toString());
        break;
      case QVariant::Map:
      case QVariant::List:
      case QVariant::BitArray:
      case QVariant::Size:
      case QVariant::SizeF:
      case QVariant::Rect:
      case QVariant::LineF:
      case QVariant::Line:
      case QVariant::RectF:
      case QVariant::Point:
      case QVariant::PointF:
      case QVariant::UserType:
      case QVariant::Invalid:
      default:
        // could not generate a hash for the given variant
        Q_ASSERT(0);
        return -1;
    }
}
