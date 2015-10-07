#ifndef GMMNODE_H
#define GMMNODE_H

#include "node/node.h"
#include "node/nodeconfig.h"
#include "rulesetdata/ruletypes.h"
#include "rulesetdata/consequent.h"
#include "rulesetdata/rule.h"
#include "rulesetdata/rulesetdata.h"
#include "tabledata/tabledata.h"
#include <QObject>
#include <QString>
#define MAX_SEGMENTS        40


class CGmmNode: public CNode
{
  Q_OBJECT

  private:
    // Data Structures
    QSharedPointer<CTableData> classif_table;
    QSharedPointer<const CTableData> gmm_train_table;
    QSharedPointer<const CTableData> gmm_test_table;
    int column_count;
    /**
    * Declaration for data training
    */
     QList<float> data_points;
     //qint32 segmentNum = 3;
     qint32 numOfData;
     float *orig_vector, *in_vector,*labs;//original_vector, input_vector, labels
     float datamin  = 1.0E30, datamax  = -1.0E30, ival, pmax, mlikedenom, p;
     double loglik_old, loglik_new,dsegment_number,conv_crit, segment_probs_total,
            mu_numer, mu_denom, sd_numer, sd_denom, loglik_sum;
     double mu[MAX_SEGMENTS],sd[MAX_SEGMENTS], segment_probs[MAX_SEGMENTS], denom[256];
     double delta_lookup[256][MAX_SEGMENTS], dnorm_lookup[256][MAX_SEGMENTS];
     int max_iterations, segment_number, empty_levels, current_breakpoint,
         initialization_error,current_total, done, initialization_multiplier,
         iter_counter,iseg;
     int sd_counter[MAX_SEGMENTS], mu_counter[MAX_SEGMENTS],  greyscale_histogram[256],
         initial_breakpoints[(MAX_SEGMENTS+1)], cardinality[MAX_SEGMENTS];
     QString tempTrainfileName = "gmmTrainClass.dat";
     QString testDatafilename = "testingData.dat";
    /**
    * Declaration for data testing
    */
    QList<float> test_data_points;
    qint32 numOfTestData;
    float *test_vector, *in_test_vector, *test_labs;
    double test_mu[MAX_SEGMENTS], test_sd[MAX_SEGMENTS],test_segment_probs[MAX_SEGMENTS];
    int test_cardinality[MAX_SEGMENTS];
    /**
    * Flags for training and testing data
    */
    bool test = false;
    bool train = false;
    bool testInFile = false;

  public:
    // Constructor
    explicit CGmmNode(const CNodeConfig &config, QObject *parent = 0);
    // Set the configuration template for this Node.
    static void configure(CNodeConfig &config);

  protected:
    // Function called when the simulation is started.
    virtual bool start();
    // Receive data sent by other nodes connected to this node.
    virtual bool data(QString gate_name, const CConstDataPointer &data);    
    // Function provides the parameters on applying the EM alogrithm for GMM.
    void trainData(const QSharedPointer<const CTableData> &table);
    // Function provides the classification of the data for GMM.
    void testData(const QSharedPointer<const CTableData> &table);
    // Function allocates space for vectors during progress.
    float* vector(int n0,int n);
    // Function creates a feature table with classification of GMM.
    bool createFeaturesTable();
    //
    void extractFeatures(const double &mean, const double &standardDev, const double &segProb,
                         const float &min, const float &max);
    // Function provides the gaussian distribution of the data
    float Normi(int x, float mu, float sd);
    float Normr(float x, float mu, float sd);
};

#endif // GMMNODE_H
