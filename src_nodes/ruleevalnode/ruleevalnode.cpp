#include "ruleevalnode.h"
#include "data/datafactory.h"
#include "data/messagedata.h"
#include "rulesetdata/ruletypes.h"
#include <QDebug>
#include <QList>
#include <cmath>


//------------------------------------------------------------------------------
// Constructor and Destructor

CRuleEvalNode::CRuleEvalNode(const CNodeConfig &config, QObject *parent/* = 0*/)
    : CNode(config, parent)
{

}


//------------------------------------------------------------------------------
// Public Functions

void CRuleEvalNode::configure(CNodeConfig &config)
{
    // Set a Description of this node.
    config.setDescription("Evaluate a table of attributes against a set "
                          "of rules to score anomalies.");

    // Add parameters
    //config.addFilename("file", "Input File", "File to be read from disk.");

    // Add the gates.
    config.addInput("in-table", "table");
    config.addInput("in-ruleset", "ruleset");
    config.addOutput("out", "table");
}


//------------------------------------------------------------------------------
// Protected Functions

bool CRuleEvalNode::start()
{
    m_anomalies_data = QSharedPointer<CTableData>(
                static_cast<CTableData *>(createData("table")));

    if(!m_anomalies_data.isNull()) {
        return true;
    }
    else {
        return false;
    }
}

bool CRuleEvalNode::data(QString gate_name, const CConstDataPointer &data)
{
    Q_UNUSED(gate_name);

    if(data->getType() == "table") {
        m_table_data = data.staticCast<const CTableData>();
        // Setup the anomalies table header by copying the header of the test table.
        m_anomalies_data->addHeader(m_table_data->header());
        // Add our own header fields.
        m_anomalies_data->addHeader("Anomaly_Score");
        m_anomalies_data->addHeader("Most_Anomalous_Rule");
        m_anomalies_data->addHeader("Rule_Score_Percentage");

        if(!m_table_data.isNull() && !m_ruleset_data.isNull()) {
            evaluate();
        }

        return true;
    }
    else if(data->getType() == "ruleset") {
        QSharedPointer<const CRulesetData> ruleset =
                data.staticCast<const CRulesetData>();
        // Clone the ruleset to our own local ruleset.
        m_ruleset_data = QSharedPointer<CRulesetData>(
                    ruleset->clone().staticCast<CRulesetData>());
        if(!m_table_data.isNull() && !m_ruleset_data.isNull()) {
            evaluate();
        }

        return true;
    }

    return false;
}

void CRuleEvalNode::evaluate()
{
    QString info,warning;
    qDebug() << "Starting the evaluation of a ruleset.";
    info="Starting the evaluation of a ruleset.";
    setLogInfo(info);

    // Do the table attribute number match the rules'?
    if(m_table_data->headerSize() != m_ruleset_data->attributeCount()) {
        qWarning() << "An evaluation cannot be performed against a"
                   << "table and a ruleset that do not match in their"
                   << "attribute count.";
        warning="An evaluation cannot be performed against a table and a ruleset that do not match in their attribute count.";
        setLogWarning(warning);                   
        return;
    }

    const double LOG10 = std::log(10);

    // Build the dataset to evaluate using the norminals of the ruleset.
    QList<QList<Nominal>> dataset;
    Antecedent tuple;

    // Set the current time of evaluation to match the number of tuples
    // ... that have already been analysed previously.
    qint32 now = m_ruleset_data->tuplesCount();

    qint32 rows = m_table_data->rowCount();
    for(qint32 i = 0; i < rows; ++i) {
        const QList<QVariant> &row = m_table_data->getRow(i);

        // Iterate each attribute of the row to build tuples.
        tuple.clear();
        tuple.reserve(m_table_data->headerSize());
        for(qint32 j = 0; j < m_table_data->headerSize(); ++j) {
            // Convert attribute to nominal.
            QString attr = row[j].toString();
            qint32 n = m_ruleset_data->string2nominal(attr);
            tuple.append(n);
        }

        // Evaluate the Tuple.
        double score = 0.0;
        double highest_score = 0.0;
        qint32 i_highest_rule = 0;
        ++now; // One more time step into the analysis.

        QList<CRule> &rules = m_ruleset_data->getRules();
        QList<CRule>::iterator it = rules.begin();
        qint32 j = 0;
        while(it != rules.end()) {
            CRule &rule= *it;
            double rule_score = 0;
            if(rule.matchAntecedents(tuple) &&
               !rule.matchConsequent(tuple) &&
               rule.consequent.valuesCount() > 0) {
                // Rule matches but the consecuent does not. Anomaly ensues.
                rule_score = double(now - rule.consequent.t) *
                             rule.consequent.n / rule.consequent.valuesCount();
                if(rule_score > highest_score) {
                    highest_score = rule_score;
                    i_highest_rule = j;
                }
                score += rule_score;
                rule.consequent.t = now;
            }
            ++it;
            ++j;
        }

        // Do something if there is an anomlay.
        double pct = 0.0;
        if(score > 0.0) {
            pct = 100 * highest_score / score;
            score = log(score) / LOG10 - 4.5;
        }
        if(score > 0.0) {
            QList<QVariant> &row = m_anomalies_data->newRow();
            for(qint32 nominal : tuple) {
                row.append(m_ruleset_data->nominal2string(nominal));
            }
            // Append extras.
            QString score_string;
            QTextStream score_stream(&score_string);
            score_stream << int(score) << ".";
            score_stream.setFieldWidth(6);
            score_stream.setFieldAlignment(QTextStream::AlignRight);
            score_stream.setPadChar('0');
            score_stream << int(score * 1000000) - int(score) * 1000000;
            row.append(score_string);
            row.append(i_highest_rule);
            row.append(pct);
        }
    }

    commit("out", m_anomalies_data);
}

