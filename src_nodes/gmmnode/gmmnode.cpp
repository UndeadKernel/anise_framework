#include "gmmnode.h"
#include "data/datafactory.h"
#include "data/messagedata.h"
#include "tabledata/tabledata.h"
#include <QDebug>
#include <QList>
#include <QFile>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

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
    config.setDescription("The Gaussian Mixture Models is one of statistically mature methods for clustering");
    //Set the category
    config.setCategory("Algorithm");

    // Add parameters
    config.addUInt("cluster_count", "Number of clusters",
                  "The number clusters of in which the data has to be classified",
                  3);
    config.addUInt("col_count", "Number of table columns",
                  "Number of columns of the table to be considered for GMM",
                  1);

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
    createFeaturesTable();
    return true;
}

bool CGmmNode::data(QString gate_name, const CConstDataPointer &data)
{
//------------------------------------------------------------------------------
// Gaussian Mixture model - Trains and Tests data
    column_count = getConfig().getParameter("col_count")->value.toInt();

    if(gate_name=="in_test"  && data->getType() == "table") {
        // Process table data.
        gmm_test_table = data.staticCast<const CTableData>();
        if(!gmm_test_table.isNull() && train==true) { //1.Train 2.Test
            testData(gmm_test_table);
            test = true;
           // qDebug() << "testing after training";
            return true;
        } else if (!gmm_test_table.isNull() && train==false){ // 1.Test 2.Train
            //__Write test data to a file when training is not yet completed__            
            QFile file(testDatafilename);
            auto test_table = data.staticCast<const CTableData>();
            qint32 rows = test_table->rowCount();
            if (file.open(QIODevice::ReadWrite))
            {
                QTextStream stream(&file);
                for (int i=0; i<rows; i++)
                {
                    const QList<QVariant> &row = test_table->getRow(i);                    
                    for(int j=0; j<column_count; j++)
                    {
                        stream << row.at(j).toString();
                        if(j != column_count - 1) {
                            stream << "\t";
                        }
                    }
                     stream << endl;
                }
            } file.close();
            testInFile = true;
            return true;
        } else {
            commitError("out", "GMM did not receive a valid testing data.");
            return false;
        }
    }

    if(gate_name=="in_train"  && data->getType() == "table") {
        // Process table data.
        gmm_train_table = data.staticCast<const CTableData>();
        qDebug()<<gmm_train_table;
        if(!gmm_train_table.isNull() && test==false && testInFile==false) { //1.Train 2.Test
            trainData(gmm_train_table);
            train = true;
            return true;
        } else if(!gmm_train_table.isNull() && test==false && testInFile==true){ //1.Test 2.Train
            trainData(gmm_train_table);
            testData(gmm_test_table);
            test = true;
        } else {
            commitError("out", "GMM did not receive a valid training data.");
            return false;
        }
    }

    if(test)
    {
        commit("out",classif_table);
        classif_table.clear();
    }
    return false;
}

/** GAUSSIAN MIXTURE MODEL. CLASSIFICATION OF TESTING DATA
 *  @brief CGmmNode::testData
 *  @param table
 */
void CGmmNode::testData(const QSharedPointer<const CTableData> &table)
{
    int i=0,k=0;
    //Output the result of test data into a text file
    QFile file("./testDataOutput.txt");
    file.open(QIODevice::WriteOnly | QIODevice::Text|QIODevice::Truncate);
    QTextStream testDataStream( &file );
    qint32 rows = 0;
    QString warning,info;
    testDataStream<<"data-point"<<"\t"<<"segment"<<"\t"<<"probability"<<"\n";
    qDebug()<<"Classifying the data as per GMM....";
    if(table)
    {
        rows = table->rowCount();
        qint32 attribute_count = column_count;
        for (int i=0; i<rows; i++)
        {
            QList<QVariant> const &row = table->getRow(i);
            for(int j=0 ; j<attribute_count; j++)
            {
               QString val = row.at(0).toString();
               test_data_points.append(val.toFloat());
            }
        }
        qDebug() << "Number of data points for classification::: "<<test_data_points.size();
        info= "Number of data points for classification::: "+QString::number(test_data_points.size());
        setLogInfo(info);
        numOfTestData = test_data_points.size();
        test_vector = vector(0,numOfTestData-1); /* Input test data */
        in_test_vector = vector(0,numOfTestData-1); /* Input data */
        test_labs = vector(0,numOfTestData-1);      /* Used later for cluster test labels */

        for (i = 0; i < numOfTestData; i++)
        {
           test_vector[i] = test_data_points[i];
        }

        for (i = 0; i < numOfTestData; i++) in_test_vector[i] = test_vector[i];

        //Read classification data obtained through training process.
        QFile gmmFile(tempTrainfileName);
        QString classifierInfo;

        if (gmmFile.open(QIODevice::ReadOnly))
        {
            classifierInfo = gmmFile.readAll();
        } gmmFile.close();gmmFile.remove();

        //Obtain mean, standard deviation and segment probabilities
        QRegExp rx("(\\n|\\;)");
        QStringList mu_Sd_SegProb = classifierInfo.split(rx,QString::SkipEmptyParts);
        int mu_count = 0, sd_count=0,seg_prob_count = 0;

        for(i=0; i<mu_Sd_SegProb.size(); i++)
        {
            if(fmod(i,3)==0)
            {
                test_mu[mu_count] = mu_Sd_SegProb[i].toDouble();
                mu_count++;
            }
            else if(fmod(i,3)==1)
            {
                test_sd[sd_count] = mu_Sd_SegProb[i].toDouble();
                sd_count++;
            }
            else if(fmod(i,3)==2)
            {
                test_segment_probs[seg_prob_count] = mu_Sd_SegProb[i].toDouble();
                seg_prob_count++;
            }
        }

        for(i=0; i<segment_number; i++)
        {
            test_cardinality[i] = 0;
        }
        for (i=0; i < numOfTestData; i++)
        {
            test_labs[i] = 0.0;
            ival = in_test_vector[i];
            pmax = 0.0;
            iseg = 0;
            mlikedenom = 0.0;
            for (k=0; k<segment_number; k++)
                 mlikedenom += Normr(ival, test_mu[k], test_sd[k]) * test_segment_probs[k];
            for (k=0; k<segment_number; k++)
            {
                p = (Normr(ival, test_mu[k], test_sd[k]) * test_segment_probs[k])/mlikedenom;
                if (p > pmax)
                {
                    pmax = p;
                    iseg = k;
                }
            }
            /* Increment cluster numbers by one so sequence is 1, 2, ... */
            test_labs[i] = iseg + 1;
            testDataStream<<ival<<"\t\t"<<iseg<<"\t"<<pmax<<"\n";
            test_cardinality[iseg] =test_cardinality[iseg]+ 1;
        }
        QString header = "Segment,    Mu,       SD,       Mix.Prob,    Card.";
        qDebug()<<"Test Labels are 1, 2, ... no. clusters.\n";
        qDebug()<<header;
        for (i=0; i<segment_number; i++)
        {
            extractFeatures(test_mu[i],test_sd[i],test_segment_probs[i],datamin,datamax);
            qDebug()<<i<<"       "<<test_mu[i]<<"  "<<test_sd[i]<<"  "
                    <<test_segment_probs[i]<<"  "<<test_cardinality[i];
        }

        qDebug() << "GMM Classification done for the data";
    }
    file.close();
}

/** GAUSSIAN MIXTURE MODEL. CLASSIFICATION OF TRAINING DATA
 *  @brief CGmmNode::trainData
 *  @param table
 *  This method classifies the training data and and dumps the classified data.
 *  The dump is to be used by the test data, for it's classification.
 */
void CGmmNode::trainData(const QSharedPointer<const CTableData> &table)
{
    // Extract the data from the table and keep it in a singular list
    int i=0,j=0,k=0;//iseg=0;
    //-----
    QString warning,info;
    qint32 rows = table->rowCount();
    qDebug() << "Number of data points for training::: "<<rows;
    qint32 attribute_count = column_count;
    for (int i=0; i<rows; i++)
    {
        QList<QVariant> const &row = table->getRow(i);
        for(int j=0 ; j<attribute_count; j++)
        {
           QString val = row.at(0).toString();
           data_points.append(val.toFloat());
        }
    }    
    //-----

    numOfData = data_points.size();
    orig_vector = vector(0,numOfData-1); /* Input data */
    in_vector = vector(0,numOfData-1); /* Input data */
    labs = vector(0,numOfData-1);      /* Used later for cluster labels */

    //Set the data points into the orig_vector
    for (i = 0; i < numOfData; i++)
    {
        orig_vector[i] = data_points[i];
    }

    //Set the data points into in_vector
    for (i = 0; i < numOfData; i++) in_vector[i] = orig_vector[i];

    // Adjustment of the minima and maxima
    for (i = 0; i < numOfData; i++)
    {
          if (in_vector[i] < datamin) datamin = in_vector[i];
          if (in_vector[i] > datamax) datamax = in_vector[i];
    }

    qDebug() << "Minima :: "<< datamin<<"   "<<"Maxima :: "<<datamax;
    info= " Minima :: " + QString::number(datamin) + "   " + "Maxima :: " + QString::number(datamax);
    setLogInfo(info);

       /* delta_lookup is prob of a datum (we're using histogram) with value j being in segment i.
       mu, sd, and segment_probs are the estimated mean, standard deviation, and mixture probability for each segment. */
    loglik_old      = 0;
    loglik_new      = 1;
    max_iterations  = 200;
    segment_number = getConfig().getParameter("cluster_count")->value.toInt();
    if(segment_number<1)
    {
        commitError("out","At least one class needs to be defined");
    }
    if(segment_number>40){
        warning= " Allowed number of classes = 1..40.";
        setLogWarning(warning);
    }

    classif_table->reserveRows(classif_table->rowCount()+segment_number);
    dsegment_number = segment_number;
    for (i=0; i<segment_number; i++)
    {
          mu[i]            = 0.0;
          sd[i]            = 0.0;
          mu_counter[i]    = 0;
          sd_counter[i]    = 0;
          segment_probs[i] = (1.0/dsegment_number);
    }

    /* Rescale values to 0..255 */
      for (i = 0; i < numOfData; i++)
          in_vector[i] = 255.0*(in_vector[i]-datamin)/(datamax-datamin);

    /* Initialize delta_lookup to the starting guess
       the starting guess is found by dividing the histogram of greyscale
       levels into equally sized pieces, so initially each segment will
       have the same segment probability.*/

    /* Find cardinality of each greyscale level (histogram bin card.) */

    empty_levels=0;

    for (i=0; i<256; i++)
    {
        greyscale_histogram[i] = 0;
        for (j=0; j<numOfData; j++)
        {
            if ((int)in_vector[j]==i) greyscale_histogram[i] += 1;
        }
    if (greyscale_histogram[i]==0) empty_levels+=1;
    }

    /* For the initial segmentation, there are a total of n values to be
       divided (roughly) equally among number_segment initial segments.
       Thus we want about n/number_segment values in each bin.  We use
       initial_breakpoints to store the breakpoints of our binning.
       Note that this will not give a good result when all the points
       are in one histogram bin.  Warning: problems if hist. sparse!
       We should check to make sure the number of unique non-zero grey
       levels (or non-empty histogram bins) is larger than segment_number. */

    initial_breakpoints[0]              = -1;
    initial_breakpoints[segment_number] = 256;
    current_breakpoint                  = -1;
    initialization_error                = 0;

    for (i=1; i<(segment_number); i++)
    {
           current_total = 0;
           done = 0;
           while (done==0)
           {
                 current_breakpoint += 1;
                 if (current_breakpoint==255) initialization_error+=1;
                 current_total += greyscale_histogram[current_breakpoint];
                 if (current_total >= (numOfData/segment_number) )
                 {
                    initial_breakpoints[i] = current_breakpoint;
                    done = 1;
                 }
           }
           if (current_total==0) initialization_error+=1;
    }

    initialization_multiplier = 1;
    while (initialization_error > 0)
    {

           initial_breakpoints[0]              = -1;
           initial_breakpoints[segment_number] = 256;
           current_breakpoint                  = -1;
           initialization_error                = 0;
           initialization_multiplier           = initialization_multiplier*2;
           for (i=1; i<(segment_number); i++)
           {
               current_total = 0;
               done = 0;
               while (done==0)
               {
                   current_breakpoint += 1;
                   if (current_breakpoint==255) initialization_error+=1;
                   current_total += greyscale_histogram[current_breakpoint];

                   if (initialization_multiplier == 0)
                   {
                       commitError("out","Pathological case in histogram. Stopping.");
                       break;
                   }

                   if (initialization_multiplier >= 4096)
                   {
                       commitError("out","Pathological case in histogram. Stopping.");
                       break;
                   }

                   if (current_total >= (int)((float)numOfData/( (float)initialization_multiplier *(float)segment_number )))
                   {
                       initial_breakpoints[i] = current_breakpoint;
                       done = 1;
                   }
                }
                if (current_total==0) initialization_error+=1;
          }
    }

    /* Check here that the init breaks are valid.  If not, probably trying
       to fit more segments than unique greyscale values (or non-zero
       histogram bins).*/
    for (i=0; i<(segment_number); i++)
    {
          for (j=0; j<256; j++)
          {
              if ( (j>initial_breakpoints[i]) &&
                   (j<= initial_breakpoints[(i+1)]))
              {
                 delta_lookup[j][i] = 1.0;
              }
              else
                  delta_lookup[j][i] = 0.0;
          }
    }
    /*End of initializing delta_lookup*/

    /* Initialize mu to the starting guess*/
    for (i=0; i<segment_number; i++)
    {
        for (j=0; j<256; j++)
        {
            if (delta_lookup[j][i] == 1.0)
            {
                mu[i] += (double)j*(double)greyscale_histogram[j];
                mu_counter[i] += greyscale_histogram[j];
            }
        }
    }

    for (i=0; i<segment_number; i++) {
           if (mu_counter[i] == 0) {
              commitError("out","Cluster empty. BIC: NA");
              break;
           }
           mu[i] = (mu[i]/(double)mu_counter[i]);
    }

    /* Initialize sd to the starting guess */
    for (i=0; i<segment_number; i++)
    {
           for (j=0; j<256; j++) {
               if (delta_lookup[j][i] == 1.0)
               {
                    sd[i] += (double)greyscale_histogram[j]*pow(((double)j-mu[i]),2);
                    sd_counter[i] += greyscale_histogram[j];
               }
           }
    }

    for (i=0; i<segment_number; i++)
    {
           sd[i] = sqrt(sd[i]/((double)sd_counter[i] - 1.0));
           /* Avoid zero std. dev. by giving instead a small value. */
           if (sd[i]==0.0) sd[i] = .25;
    }


    /* Use EM algorithm to find MLE for segment_number segments.
       Note that the first step will have a negative change in
       loglikelihood since the first loglik_old value is zero. */

    iter_counter = 0;

    /* Set the convergence criterion based on the number of observations.
       We want the average change in loglikelihood per val. to be less than
       .00001, so we multiply the number of values by .00001 to get the
       convergence criterion (with an upper bound of 1). */

    conv_crit = (double)numOfData*(0.00001);
    if (conv_crit>1.0) conv_crit=1.0;
    qDebug() << "Convergence criterion :: " << conv_crit;

    /* Main while loop ************* */
    while ( (fabs(loglik_new-loglik_old) > (float)conv_crit) ||
            (iter_counter==0) )
    {
        iter_counter += 1;

        /* E-step */

        /* Only need to calculate the Gaussian probs. for values 0-255 for each segment*/
        for (i=0; i<segment_number; i++)
        {
            for (j=0; j<256; j++)
                dnorm_lookup[j][i] = segment_probs[i] *Normi(j,mu[i],sd[i]);
        }

        for (j=0; j<256; j++) denom[j] = 0.0;

        for (i=0; i<segment_number; i++)
        {
            for (j=0; j<256; j++) denom[j] = denom[j] + (dnorm_lookup[j][i]);
        }

        /* if denom[j]=0, then there are no points with greyscale level j,
           so delta_lookup will never be called for that value of j unless
           multiplied by greyscale histogram[j] which will be zero. */

        for (i=0; i<segment_number; i++)
        {
            for (j=0; j<256; j++)
            {
                if (denom[j]>0)
                    delta_lookup[j][i] = dnorm_lookup[j][i]/denom[j];
                else
                    delta_lookup[j][i] = 0;
            }
        }

        /* M-step */

        if (fmod(iter_counter,1000)==0)
                   qDebug() <<"Another 1000 iterations...";

        /* update segment_probs */
        for (i=0; i<segment_number; i++) segment_probs[i] = 0.0;

        for (i=0; i<segment_number; i++)
        {
                    for (j=0; j<256; j++)
                        segment_probs[i]+=(delta_lookup[j][i])*greyscale_histogram[j];
        }

        for (i=0; i<segment_number; i++)
                    segment_probs[i] = segment_probs[i]/(float)numOfData;

        segment_probs_total = 0.0;
        for (i=0; i<segment_number; i++)
                    segment_probs_total += segment_probs[i];

        for (i=0; i<segment_number; i++)
                 segment_probs[i] = segment_probs[i]/segment_probs_total;

        /* Update mu */

        for (i=0; i<segment_number; i++)
        {
            mu_numer = 0.0;
            mu_denom = 0.0;
            for (j=0; j<256; j++)
            {
                mu_numer += (delta_lookup[j][i])*(j*greyscale_histogram[j]);
                mu_denom += (delta_lookup[j][i])*greyscale_histogram[j];
            }
            mu[i] = mu_numer/mu_denom;
        }

        /* Update sd */
        for (i=0; i<segment_number; i++)
        {
            sd_numer = 0.0;
            sd_denom = 0.0;
            for (j=0; j<256; j++)
            {
                sd_numer += greyscale_histogram[j]*pow((j-mu[i]),2)*delta_lookup[j][i];
                sd_denom += greyscale_histogram[j]*delta_lookup[j][i];
            }

            sd[i] = sqrt(sd_numer/sd_denom);

            /* We know that the resolution of our values cannot be better than 1 greyscale level,
             * so sd should not fall below 1/4. */
            if (sd[i]<.25) { sd[i] = .25; }
        }

        /* Segment probs have been updated, so now update dnorm_lookup*/
        for (i=0; i<segment_number; i++)
        {
            for (j=0; j<256; j++)
                dnorm_lookup[j][i] = segment_probs[i] *Normi(j,mu[i],sd[i]);
        }

        /* Update likelihood */
        loglik_old = loglik_new;
        loglik_new = 0.0;

        for (j=0; j<256; j++)
        {
            loglik_sum = 0.0;
            for (i=0; i<segment_number; i++)
                        loglik_sum += dnorm_lookup[j][i];
            if (loglik_sum>0)
                loglik_new += greyscale_histogram[j]*log(loglik_sum);
                /* Otherwise, greyscale_histogram will be zero, so add zero. */
        }
    }

    /* Classify each data point into its most likely segment */
    qDebug() << "Loglikelihood:: "<<loglik_new;
    qDebug()<< "Now, determining the posterior of each data point ";

    /* Determine posteriors.  By Bayes: prob(seg-k | ival) = f(ival | seg-k) . prob(seg-k) /
     * sum_r f(ival | seg-r) . prob(seg-r)
     * where LHS is posterior probability of seg-k given the data, ival,
     * if(ival | seg-k) is the integrated or marginal likelihood of seg k
     * prob(seg-k) is prior probability */

    for (k = 0; k < segment_number; k++) cardinality[k] = 0;

    for (i=0; i < numOfData; i++)
    {
        labs[i] = 0;
        ival = in_vector[i];
        pmax = 0.0;
        iseg = 0;
        mlikedenom = 0.0;
        for (k=0; k<segment_number; k++)
            mlikedenom += Normr(ival, mu[k], sd[k]) * segment_probs[k];
        for (k=0; k<segment_number; k++)
        {
            p = (Normr(ival, mu[k], sd[k]) * segment_probs[k])/mlikedenom;
            if (p > pmax)
            {
                pmax = p;
                iseg = k;
            }
        }
        /* Increment cluster numbers by one so sequence is 1, 2, ... */
        labs[i] = iseg + 1;
        cardinality[iseg] += 1;
    }

    /* For convenience, redo stdevs from original data */
    for (j=0; j<segment_number; j++)
    {
        /* Use p for convenience as rectified mean */
        p = ((float)mu[j]*(datamax-datamin)/255.0)+datamin;
        sd[j] = 0.0;
        for (i=0; i< numOfData; i++)
        {
            if (labs[i] == j+1)
            {
                ival = (in_vector[i]*(datamax-datamin)/255.0)+datamin;
                sd[j] += pow(ival-p,2);
            }
        }
        sd[j] = sqrt(sd[j]/(float)cardinality[j]);
    }
    //Classification information written temporarily into a dat file to be used for test data
    // classification process.
    QFile gmmTrainFile(tempTrainfileName);
    QTextStream stream(&gmmTrainFile);
    qDebug() << "Labels are 1, 2, ... no. clusters::"<<segment_number;
    QString header = "Segment,    Mu,       SD,       Mix.Prob,    Card.";
    qDebug() << header <<endl;
    if (gmmTrainFile.open(QIODevice::ReadWrite))
    {
        for (i=0; i<segment_number; i++)
        {
          qDebug()<<i<<"       "<<(mu[i]*(datamax-datamin)/255.0)+datamin<<"  "
                  <<sd[i]<<"  "<<segment_probs[i]<<"  "<<cardinality[i];
          stream <<(mu[i]*(datamax-datamin)/255.0)+datamin<<";"
                <<sd[i]<<";"<<segment_probs[i]<<endl;
        }
    }
    gmmTrainFile.close();
}

extern float* CGmmNode::vector(int n0, int n)
{
    float *val;
    val = (float *) malloc ((unsigned) (n-n0+1)*sizeof(float));
    return val;
}

extern float CGmmNode::Normi(int x, float mu, float sd)
{
    /* Used to compute the Gaussian prob. of integer x. */
             float result,temp;
             temp = (float)x-mu;
             result = (1.0/(sd*sqrt(2*3.141593)))*
                         exp(- pow(temp,2)/(2*pow(sd,2)));
             return(result);
}

extern float CGmmNode::Normr(float x, float mu, float sd)
{
    /* Used to compute the Gaussian probability of real x. */
             float result, temp;
             temp = x-mu;
             result = (1.0/(sd*sqrt(2.0*3.141593)))*
                          exp(- pow(temp,2)/(2*pow(sd,2)));
             return(result);
}

bool CGmmNode::createFeaturesTable()
{
    classif_table = QSharedPointer<CTableData>(
                static_cast<CTableData *>(createData("table")));

    if(!classif_table.isNull())
    {
        classif_table->addHeader("Mu");
        classif_table->addHeader("Sd");
        classif_table->addHeader("segProb");
        classif_table->addHeader("dataMin");
        classif_table->addHeader("dataMax");

        return true;
    } else
    {
        return false;
    }
}

void CGmmNode::extractFeatures(const double &mean, const double &standardDev, const double &segProb,
                               const float &min, const float &max)
{
    QList<QVariant> &classificationData = classif_table->newRow();
    classificationData<<mean;
    classificationData<<standardDev;
    classificationData<<segProb;
    classificationData<<min;
    classificationData<<max;
}
