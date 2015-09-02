#include "gmmnode.h"
#include "data/datafactory.h"
#include "data/messagedata.h"
#include "tabledata/tabledata.h"
#include <QDebug>
#include <QList>
#include <QFile>


//------------------------------------------------------------------------------
// Constructor and Destructor

CGmmNode::CGmmNode(const CNodeConfig &config, QObject *parent/* = 0*/)
    : CNode(config, parent)
{

}


//------------------------------------------------------------------------------
// Public Functions

void CGmmNode::configure(CNodeConfig &config)
{
    // Set the node Description
    config.setDescription("The LERAD Anomaly Detection Algorithm: "
                          "Build rules that describe the normal pattern "
                          "of given data.");

    // Add parameters
    //config.addFilename("file", "Input File", "File to be read from disk.");
    config.addUInt("rseed", "Random Seed", "The random seed to feed the "
                   "random number generator at the start.", 666);
    config.addUInt("sample_size", "Training Samples", "Number of samples to use "
                   "for training GMM.", 100);
    config.addUInt("pairs_to_match", "Number of Pairs to Match",
                   "Number of pairs to match for building rules", 1000);
    config.addUInt("max_rules_per_pair", "Maximum Rules per Pair",
                   "Maximum number of rules to create for each pair.", 4);
    config.addBool("dump_rules","Dump Generated Rules","Write the rules "
                   "generated by GMM to a file.", false);
    config.addFilename("rules_file", "Rules Filename", "File where the "
                       "Gaussian Mixture model are written.");

    // Add the gates.
    //config.addInput("in", "table");
    config.addInput("in_train","table");
    config.addInput("in_test","table");
    config.addOutput("out", "table");
}


//------------------------------------------------------------------------------
// Protected Functions

bool CGmmNode::start()
{
    m_ruleset = QSharedPointer<CRulesetData>(
        static_cast<CRulesetData *>(createData("ruleset")));

    if(!m_ruleset.isNull()) {
        return true;
    }
    else {
        return false;
    }
}

bool CGmmNode::data(QString gate_name, const CConstDataPointer &data)
{
    //Q_UNUSED(gate_name);
//------------------------------------------------------------------------------
// Gaussian Mixture model - train and test data
    bool train = false;
    bool test  = false;

    if(gate_name=="in_train"  && data->getType() == "table") {
        // Process table data.
        gmm_train_table = data.staticCast<const CTableData>();
        if(!gmm_train_table.isNull()) {
            trainData(gmm_test_table);
            train = true;
        } else {
            commitError("out", "GMM did not receive a valid training data.");
        }
    }

    if(gate_name=="in_test"  && data->getType() == "table") {
        // Process table data.
        gmm_test_table = data.staticCast<const CTableData>();
        if(!gmm_test_table.isNull() && train==true) {
            testData(gmm_test_table);
            test = true;
        } else {
            commitError("out", "GMM did not receive a valid testing data.");
        }
    }

    if(!gmm_train_table.isNull() && !gmm_test_table.isNull()) {
        if(!train) {
            trainData(gmm_train_table);
            testData(gmm_test_table);
        } else if(train && !test){
            testData(gmm_test_table);
        }
    }

    if(train && test) {
        return true;
    } else {
        return false;
    }
}

//------------------------------------------------------------------------------
//Testing algorithm
void CGmmNode::testData(const QSharedPointer<const CTableData> &table)
{
    qDebug()<<"Test Data";
}

//------------------------------------------------------------------------------
//Training algorithm
void CGmmNode::trainData(const QSharedPointer<const CTableData> &table)
{
    qDebug()<<"GMM Data training started...";
    // Number of attributes
    qint32 attribute_count = table->colCount();  // Col 1 --> Time Col 15 --> Length
    qint32 rows = table->rowCount();
    int prev_hour = 0;
    int prev_min  = 0;
    int prev_sec  = 0;
    ulong timeDiff = 0.01;//initial offset

    for(qint32 j = 0; j < rows; ++j)
    {
        ulong rate = 0;
        // Get the row.
        const QList<QVariant> &row = table->getRow(j);
         for(qint32 i = 0; i < attribute_count; ++i)
         {
             if(i==1)
             {
                 QString time = row[i].toString();
                 QStringList timeList = time.split(":");
                 int currenthour = timeList[0].toInt();
                 int currentmin  = timeList[1].toInt();
                 int currentsec  = timeList[2].toInt();

                 if(j!=0) {
                     uint hour = currenthour - prev_hour;
                     uint min  = currentmin  - prev_min;
                     uint sec  = currentsec  - prev_sec;

                     timeDiff = hour*3600 + min*60 + sec;
                     //qDebug() << timeDiff;
                     prev_hour = currenthour;
                     prev_min  = currentmin;
                     prev_sec  = currentsec;
                 }

                 if(j==0) {
                     prev_hour = currenthour;
                     prev_min  = currentmin;
                     prev_sec  = currentsec;
                 }

                 if(i==attribute_count-9)
                 {
                     uint length = row[i].toInt();
                     //rate = length/time
                     if(timeDiff==0)
                     {
                         //use initial offset
                         rate = length/0.01;
                         qDebug()<< "offset_rate"<<rate;

                     } else
                     {
                         //use current time period
                         rate = length/timeDiff;
                         qDebug()<< "current_rate"<<rate;
                     }
                 }
             }
         }

    }

}
