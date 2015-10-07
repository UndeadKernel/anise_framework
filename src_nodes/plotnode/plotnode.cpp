#include "plotnode.h"
#include "data/datafactory.h"
#include "data/messagedata.h"
#include "tabledata/tabledata.h"
#include <QProcess>
#include <QDebug>
#include <QFile>


//------------------------------------------------------------------------------
// Constructor and Destructor
QProcess *CPlotNode::anise_process = new QProcess();

CPlotNode::CPlotNode(const CNodeConfig &config, QObject *parent/* = 0*/)
    : CNode(config, parent)
{

}


//------------------------------------------------------------------------------
// Public Functions

void CPlotNode::configure(CNodeConfig &config)
{
    // Set a Description of this node.
    config.setDescription("Plots numberical data onto a graph or plots it from a file");
    //Set a Category for this node.
    config.setCategory("Plot");
    config.addBool("guassian_plot", "Plots for gaussian data",
                   "Can plot gaussian curves for GMM, else will be a 1D scatter plot",
                   true);
    config.addString("file_name", "Name of the png file",
                     "The graphical information"
                     "for the analysis.");
    // Add the gates.
    config.addInput("in", "table");
    //config.addOutput("out", "misc");
}


//------------------------------------------------------------------------------
// Protected Functions

bool CPlotNode::start()
{
    qDebug() << "Start called.";
    return true;
}

bool CPlotNode::data(QString gate_name, const CConstDataPointer &data)
{
    Q_UNUSED(gate_name);
    QVector<QString> x(101), y(101);

    // Process framework messages.
    if(data->getType() == "table") {
        // Process table data.
        auto table = data.staticCast<const CTableData>();
        qint32 rows = table->rowCount();
        //qint32 attribute_count = table->colCount();
        QString filename = "plotInfo.dat";
        QFile file(filename);

        QString terminalSet = "set terminal pngcairo\n";
        //QString file_name = getConfig().getParameter("file_name")->value.toString();


        if(getConfig().getParameter("guassian_plot")->value.toBool())
        {
            qint32 col_count = table->colCount();
            for (int i=0; i<rows; i++)
            {
                QList<QVariant> const &gausses = table->getRow(i);
                //mean.append(gausses[col_count-col_count].toString());
                //standard_deviation.append(gausses[col_count-(col_count-1)].toString());
                //datamin.append(gausses[col_count-1].toString());
                //datamax.append(gausses[col_count].toString());
                QString segment = "normal(x," + gausses[col_count-col_count].toString()
                        + "," + gausses[col_count-(col_count-1)].toString() + ","
                        + gausses[col_count-(col_count-1)+1].toString() + ")";
                QString line = "normal(x," + gausses[col_count-col_count].toString()
                        + "," + gausses[col_count-(col_count-1)].toString() + ")";
                plot_lines.append(line);
                plot_gmm.append(segment);
            }

            for (int i=0; i<plot_lines.size(); i++)
            {
                if(!normals.isEmpty())
                {
                    normals = normals+","+plot_lines[i];
                    gmm = gmm + "+" + plot_gmm[i];

                } else if(normals.isEmpty())
                {
                    normals = plot_lines[i];
                    gmm = plot_gmm[i];
                }

            }

            QList<QVariant> const &min_max = table->getRow(0);
            datamin = min_max[col_count-2].toString();
            datamax = min_max[col_count-1].toString();

            QString gPlot = terminalSet+
                    "set output 'gauss_plot.png'\n"
                    "set clip two\n"
                    "set style fill  transparent solid 0.50 noborder\n"
                    "set key title 'Gaussian Distributions'\n"
                    "set key noinvert samplen 1 spacing 1 width 0 height 0 \n"
                    "set style data lines\n"
                    "set style function filledcurves y1=0\n"
                    "set boxwidth 0.1\n"
                    "set xrange"+"["+datamin+":"+datamax+"]\n"
                    "normal(x, mu, sd) = (1/(sd*sqrt(2*pi)))*exp(-(x-mu)**2/(2*sd**2))\n"
                    "plot "+ normals;
            QString gPlot_1 = terminalSet+
                    "set output 'gmm_plot.png'\n"
                    "set boxwidth 0.1\n"
                    "set xrange"+"["+datamin+":"+datamax+"]\n"
                    "normal(x, mu, sd, prob) = prob*(1/(sd*sqrt(2*pi)))*exp(-(x-mu)**2/(2*sd**2))\n"
                    "plot "+ gmm + "title 'Gaussian Mixture Model'";
                    // "normal(x,-0.25,0.95),normal(x,1.5,0.05)";
            QString exe = "gnuplot";
            QString plotFileName = "guassPlot.gnu";
            QFile plotFile(plotFileName);
            if (plotFile.open(QIODevice::ReadWrite))
            {
                    QTextStream stream(&plotFile);
                    stream << gPlot << endl;
            }
            QString cmd = exe + " " + plotFileName;
            anise_process->start(cmd);
            anise_process->waitForFinished();
            plotFile.close();
            QString plotFileName_1 = "gmmPlot.gnu";
            QFile plotFile_1(plotFileName_1);
            if (plotFile_1.open(QIODevice::ReadWrite))
            {
                    QTextStream stream(&plotFile_1);
                    stream << gPlot_1 << endl;
            }
            QString cmd_1 = exe + " " + plotFileName_1;
            anise_process->start(cmd_1);
            anise_process->waitForFinished();
            plotFile_1.close();
        } else
        {
            QStringList output;
            for (int i=0; i<rows; i++)
            {
                QList<QVariant> const &row = table->getRow(i);
                x[i] = row[0].toString();
                y[i] = row[1].toString();
                QString line = x[i]+" "+y[i];
                output.append(line);
            }
            if (file.open(QIODevice::ReadWrite))
            {
                    QTextStream stream(&file);
                    for(QString str:output){
                    stream << str << endl;
                    }
            }
            QString exe = "gnuplot";
            QString plotFileName = "test.txt";
            QFile plotFile(plotFileName);
            QString sPlot =
                           "set output 'scatter_plot.png'\n"
                           "set key inside left top vertical Right noreverse enhanced autotitles box linetype -1 linewidth 1.000\n"
                           "set samples 50, 50\n"
                           "plot \"test.dat\" using 1:2 title 'Scatter Plot'";
            if (plotFile.open(QIODevice::ReadWrite))
            {
                    QTextStream stream(&plotFile);
                    stream << sPlot << endl;
            }
            QString cmd = exe + " " + plotFileName;
            anise_process->start(cmd);
            anise_process->waitForFinished();
            file.close();
            plotFile.close();
            file.remove();
            plotFile.remove();
        }
        return true;
    }
    return false;
}
