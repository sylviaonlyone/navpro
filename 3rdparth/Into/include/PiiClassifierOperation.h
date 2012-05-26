/* This file is part of Into.
 * Copyright (C) 2003-2011 Intopii.
 * All rights reserved.
 *
 * IMPORTANT LICENSING INFORMATION
 *
 * Into is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License version 3
 * as published by the Free Software Foundation.
 *
 * Into is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef _PIICLASSIFIEROPERATION_H
#define _PIICLASSIFIEROPERATION_H

#include <PiiDefaultOperation.h>
#include "PiiClassificationGlobal.h"
#include "PiiClassification.h"
#include "PiiClassifier.h"
#include "PiiLearningAlgorithm.h"
#include <QMutex>

class QThread;

namespace PiiClassification
{
  /**
   * @internal
   *
   * A model for feature readers. Copy this if you implement your own
   * sample sets.
   */
  template <class SampleSet> struct FeatureReader;
  /*
  {
    typedef typename PiiSampleSet::Traits<SampleSet>::ConstFeatureIterator ConstFeatureIterator;
    
    ConstFeatureIterator operator() (PiiInputSocket* input, int* featureCount)
    { 
    }
  };
  */

  /**
   * A function object that reads an input socket and converts the
   * incoming object to a PiiMatrix<T>, if possible.
   */
  template <class T> struct FeatureReader<PiiMatrix<T> >
  {
    typedef const T* ConstFeatureIterator;

    /**
     * Reads @a input and converts the incoming object to a
     * PiiMatrix<T>, if possible.
     *
     * @param input the input socket to read
     *
     * @param featureCount the number of features to expect. This is
     * an input-output parameter that must be initially set to the
     * number of features to expect. If this value is zero, no checks
     * will be performed. Upon return, @a featureCount will be set to
     * the number of columns in the input matrix.
     *
     * @return a pointer to the beginning of the feature vector (first
     * row of matrix).
     *
     * @throw PiiExecutionException& if the input object is not a
     * PiiMatrix or if its size is not 1-by-@a featureCount.
     */
    ConstFeatureIterator operator() (PiiInputSocket* input, int* featureCount)
    { 
      matFeatures = PiiYdin::convertMatrixTo<T>(input);
      if (matFeatures.rows() != 1 || (*featureCount > 0 && matFeatures.columns() != *featureCount))
        PII_THROW_WRONG_SIZE(input, matFeatures, 1, *featureCount);
      *featureCount = matFeatures.columns();
      return matFeatures[0];
    }
    PiiMatrix<T> matFeatures;
  };
};

/**
 * A superclass for classifier operations. This operation provides
 * standard classification and learning facilities. In classification,
 * a classification algorithm (usually a class derived from
 * PiiClassifier) is used to map incoming feature vectors to real
 * numbers. In learning, the operation will collect samples until the
 * learning algorithm (usually a class derived from
 * PiiLearningAlgorithm) is started.
 *
 * Learning is usually an off-line process in which a batch of samples
 * is first collected and a learning algorithm is applied to it. 
 * Certain algorithms such as the SOM are also capable of incremental
 * (on-line) learning.
 *
 * The #learningBatchSize property is used as a
 * learning/classification switch. Setting its value to zero disables
 * learning and turns the operation into classification mode. If the
 * learning algorithm is capable of on-line learning and
 * #learningBatchSize is set to one, each incoming sample will be
 * directly sent to learning.
 *
 * Batch learning must be initiated by the user by calling the
 * #startLearningThread() function. Although learning is usually done
 * off-line, it is possible to start the learning thread while the
 * operation is running. The old classifier will be replaced by the
 * new one once the learning thread finishes. The downside of run-time
 * learning is that the old classifier must be kept in memory while
 * training. If you want to avoid this, @ref reset() "reset" the old
 * classifier before learning.
 *
 * @inputs
 *
 * @in features - a feature vector. Features are usually represented
 * as a row matrix with a primitive content type (such as
 * PiiMatrix<double>), but subclasses are free to use any feature
 * representation appropriate for the task in hand.
 *
 * @in label - a label for the corresponding feature vector (double). 
 * This input is ignored by non-supervised classifiers (such as the
 * SOM). In supervised classifiers (such as k-NN), the input can be
 * left unconnected in classification, but not in learning.
 *
 * @in weight - an optional weight for the training sample. This input
 * will be used only by learning algorithms that are capable of
 * weighted learning, and only in training mode. If this input is not
 * connected, a weight of 1.0 is assumed for all samples.
 *
 * @outputs
 *
 * @out classification - the result of classification (double). Either
 * a class index or a regression. @p NaN indicates failures.
 *
 * The usual way of creating a custom classifier is to first create an
 * operation class that reflects the configuration of the classifier
 * in its properties and uses pure virtual getter and setter functions
 * for each. Then, an inner template class called "Template" is
 * derived from this operation class and instantiated for @p float and
 * @p double types. The reason for this design pattern is that Qt's
 * moc can't cope with template classes. This pattern is implemented,
 * for example, in PiiKnnClassifierOperation.
 *
 * The template classes are registered to the @ref
 * PiiYdin::resourceDatabase() "resource database" so that the
 * template type is a part of the name. For example,
 * PiiKnnClassifierOperation::Template<double> is registered as
 * PiiKnnClassifierOperation<double>.
 *
 * @note Once the first feature vector has been received, the number
 * of features in subsequent feature vectors must stay the same. One
 * needs to explicitly reset the classifier before samples with a
 * different number of features can be used. If the operation has
 * training data, both the current classifier and the collected
 * training data must be cleared. This is done by calling #reset() and
 * setting #learningBatchSize to zero.
 *
 * @ingroup PiiClassificationPlugin
 */
class PII_CLASSIFICATION_EXPORT PiiClassifierOperation :
  public PiiDefaultOperation,
  public PiiProgressController
{
  Q_OBJECT

  /**
   * A read-only property that specifies the capabilities of the
   * learning algorithm.
   */
  Q_PROPERTY(PiiClassification::LearnerCapabilities capabilities READ capabilities);
  
  /**
   * The maximum number of training samples collected for learning. 
   * This property is also used as a training/classification switch. 
   * Setting the value to zero means that no training samples will be
   * collected, and the operation will only classify incoming samples. 
   * If @p learningBatchSize is set to one and the learning algorithm
   * is capable of on-line learning, incoming samples will be used to
   * train the algorithm one by one. If @p learningBatchSize is set to
   * N (N > 1), a buffer of N first, last or randomly selected samples
   * will be kept in memory. If @p learningBatchSize is -1, all
   * incoming samples will be buffered without a limit. The buffered
   * samples will be used as training data to the learning algorithm,
   * which will be run in a separate thread (see
   * #startLearningThread()). The default value is 0.
   */
  Q_PROPERTY(int learningBatchSize READ learningBatchSize WRITE setLearningBatchSize);

  /**
   * The action to perform with new samples when #learningBatchSize has been
   * exceeded. The default is @p OverwriteRandomSample.
   */
  Q_PROPERTY(PiiClassification::FullBufferBehavior fullBufferBehavior
             READ fullBufferBehavior WRITE setFullBufferBehavior);

  /**
   * Progress required to emit the #progressed() signal. Must be set
   * to a value in [0,1]. Set to 0 to disable the signal. Set to 1 to
   * send the signal only after training is complete. The default is
   * 0.01, which means that every percent of progress will be reported
   * (unless the training algorithm advances faster and checks the
   * condition e.g. only every two percents).
   */
  Q_PROPERTY(double progressStep READ progressStep WRITE setProgressStep);

  /**
   * The number of samples currently buffered.
   */
  Q_PROPERTY(int bufferedSampleCount READ bufferedSampleCount);

  /**
   * The number of features in each sample. Initially, this value is
   * unknown (0). This property will be set when the first sample is
   * sent to the learning algorithm.
   */
  Q_PROPERTY(int featureCount READ featureCount);

  /**
   * A read-only property whose value is @p true when the learning
   * thread is running, and @p false otherwise.
   */
  Q_PROPERTY(bool learningThreadRunning READ learningThreadRunning);

  /**
   * A textual description of a learning error.
   */
  Q_PROPERTY(QString learningError READ learningError);  

public:
  /**
   * Destroys the operation. The operation will not be destructed
   * until the learning thread has finished.
   */
  ~PiiClassifierOperation();

  /**
   * If @a reset is @p true and the learning thread is running, this
   * function stops it. Otherwise just passes the call to the
   * superclass.
   */
  void check(bool reset);

  /**
   * Implementation of the PiiProgressController interface. This
   * function is called by learning algorithms to check if they are
   * still allowed to proceed. This function returns @p true if
   * #startLearningThread() has been called and #stopLearningThread()
   * has not been called. It also emits the #progressed() signal if @a
   * progressPercentage is not @p NaN and it is #progressStep units
   * larger than the previous recorded progress.
   */
  bool canContinue(double progressPercentage);
  
public slots:
  /**
   * Start the learning thread. If the number of buffered samples is
   * less than two or the learning thread is already running, this
   * function does nothing. Otherwise, it starts a thread that sends
   * the buffered samples to the learning algorithm (see
   * #learnBatch()). The thread can be interrupted by calling
   * #stopLearningThread().
   *
   * The learning thread will stop once #learnBatch() returns.
   *
   * @return @p true if the learning thread was successfully started,
   * @p false otherwise. The call will fail if the learning thread is
   * already running or there are no buffered samples to learn.
   */
  bool startLearningThread();

  /**
   * Stop the learning thread. After this function has been called,
   * #canContinue() will return @p false, which interrupts the
   * learning algorithm.
   */
  void stopLearningThread();

  /**
   * Resets the classifier. This function clears all training results
   * and resets the classifier to its initial state. It should not
   * change classifier configuration though. This function serves as
   * as generic way of resetting a trained classifier. Subclasses may
   * provide other features that have the same effect (such as setting
   * the model samples of an NN classifier to an empty sample set).
   *
   * To clear buffered training data as well, set the
   * #learningBatchSize property to zero.
   */
  void reset();

  /**
   * Learns the batch of collected samples. This blocks until the
   * learning algorithm finishes.
   *
   * @return @p true if the samples were successfully learnt, @p false
   * otherwise
   */
  bool learn();

signals:
  /**
   * Informs about the progress of a learning algorithm. This signal
   * will be emitted from the learning thread started with
   * #startLearning() every time #progressStep is exceeded. Note that
   * if the learning algorithm is not capable of estimating its
   * progress, this signal will not be emitted until it is done.
   *
   * @param percentage the current progress of the learning algorithm. 
   * 0 means uninitialized, 1.0 means fully converged. Note that the
   * learning thread may not finish immediately after signaling 1.0. 
   * use the #learningFinished() signal to find out when it is done.
   */
  void progressed(double percentage);

  /**
   * This signal is emitted when the learning thread finishes.
   *
   * @param success @p true if the learning was successful, @p false
   * otherwise. If an error occurs during training, the #learningError
   * property may provide a textual error message.
   */
  void learningFinished(bool success);
  
protected:
  /// @internal
  class PII_CLASSIFICATION_EXPORT Data : public PiiDefaultOperation::Data
  {
  public:
    Data(PiiClassification::LearnerCapabilities capabilities);
    
    PiiInputSocket* pFeatureInput;
    PiiInputSocket* pLabelInput;
    PiiInputSocket* pWeightInput;
    PiiOutputSocket* pClassificationOutput;
    
    PiiClassification::LearnerCapabilities capabilities;
    int iLearningBatchSize;
    PiiClassification::FullBufferBehavior fullBufferBehavior;
    double dProgressStep;
    double dCurrentProgress;
    QThread* pLearningThread;
    QMutex learningMutex;
    bool bThreadRunning;
    QString strLearningError;
  };
  PII_D_FUNC;

  /**
   * Constructs a new classifier operation.
   */
  PiiClassifierOperation(PiiClassification::LearnerCapabilities capabilities);

  /// @internal
  PiiClassifierOperation(Data* data);

  PiiClassification::LearnerCapabilities capabilities() const;
  bool learningThreadRunning() const;
  QString learningError() const;
  
  /**
   * Returns the number of samples currently in buffer. Must be
   * implemented by subclasses to return the number of samples
   * currently in buffer.
   */
  virtual int bufferedSampleCount() const = 0;

  /**
   * Returns the number of features the classifier/learning algorithm
   * is expecting. If no feature vectors have been seen so far, zero
   * will be returned. Note that once the first sample have been
   * received, the number of features in subsequent feature vectors
   * must be the same.
   */
  virtual int featureCount() const = 0;

  /**
   * Classifies an incoming feature vector (see #classify()). If
   * #learningBatchSize is set to a non-zero value, and if the
   * learning thread is not running, collects the incoming sample to a
   * buffer (see #collectSample()). If #learningBatchSize is set to
   * one and the learning algorithm is capable of on-line learning,
   * the incoming sample will be sent directly to learning (see
   * #learnOne()).
   */
  void process();

  /**
   * Returns @p true if the learning algorithm needs a learning
   * thread, and @p false otherwise. Some classifiers such as simple
   * linear-search nearest neighbors don't need to be trained. In such
   * a case this function returns @p false, no learning thread will be
   * started, and the old classifier is immediately replaced by a new
   * one. The default implementation returns @p true.
   */
  virtual bool needsThread() const;
  
  /**
   * Trains a learning algorithm with the collected set of samples. 
   * This function is called by the learning thread and must be
   * overridden by subclasses to feed the buffered samples to the
   * learning algorithm. Typically, subclasses call the
   * #learnBatch(PiiLearningAlgorithm<SampleSet>*, const SampleSet&,
   * const QVector<double>&, const QVector<double>&) function
   * template.
   *
   * This function should not modify the currently operating
   * classifier. If the learning thread is started while the operation
   * is running, the normal functioning should not be changed. The old
   * classifier must be replaced with the newly trained one in
   * replaceClassifier().
   *
   * @return @p true if the training succeeded, @p false otherwise.
   *
   * The default implementation returns @p false.
   */
  virtual bool learnBatch();

  /**
   * Replaces the current classifier with a newly trained one. This
   * function is called if #learnBatch() returns @p true. If the
   * classifier provides information about itself as properties (such
   * as the code book of an NN classifier), these property values need
   * to be changed here.
   */
  virtual void replaceClassifier() = 0;

  /**
   * Resizes the batch of buffered samples. This function is called by
   * #setLearningBatchSize() after ensuring mutual exclusion with the learning
   * thread. The function will only be called if needed. If the new
   * size is not smaller the current number of buffered samples,
   * nothing needs to be done. The buffer will grow automatically to
   * the target size when new samples are read.
   *
   * @param newSize the new size of the batch. The batch must be
   * truncated to the requrested size.
   */
  virtual void resizeBatch(int newSize) = 0;

  /**
   * Resets the classifier. This function is called by #reset() after
   * ensuring mutual exclusion with the learning thread.
   */
  virtual void resetClassifier() = 0;

  /**
   * A template function that installs @p this as the progress
   * controller to @a algorithm and feeds it with the given @a samples
   * and @a labels. Usually called from the implementation of the
   * virtual #learnBatch() function.
   *
   * @param algorithm the algorithm to train
   *
   * @param samples training samples
   *
   * @param labels labels for the samples, if applicable. An empty
   * list can be provided for non-supervised learning algorithms.
   *
   * @param weights an importance factor for each sample. May be
   * empty, in which case 1.0 will be used for all samples.
   *
   * @return @p true if the algorithm was successfully trained, @p
   * false otherwise.
   */
  template <class SampleSet> bool learnBatch(PiiLearningAlgorithm<SampleSet>& algorithm,
                                             const SampleSet& samples,
                                             const QVector<double>& labels,
                                             const QVector<double>& weights = QVector<double>());

  /**
   * Reads a feature vector from the @p features input and emits its
   * classification to the @p classification output. May also send
   * additional objects through other output sockets. This function is
   * called by #process() when an incoming sample must be classified.
   *
   * @return the classification
   */
  virtual double classify() = 0;

  /**
   * Reads a feature vector from the @p features input, sends it to an
   * on-line learning algorithm, and emits the classification result
   * to the @p classification output. May send additional objects
   * through other output sockets. The default implementation emits
   * and returns @a label.
   *
   * This function is called by #process() when an incoming sample
   * must be used for on-line learning (only if the learning algorithm
   * is capable of on-line learning).
   *
   * @param label the classification of the training sample, or @p NaN
   * if not applicable.
   *
   * @param weight the importance of the sample. If the @p weight
   * input is not connected, this value will always be 1.0. Learning
   * algorithms that are not capable of weighted learning will ignore
   * this value.
   *
   * @return the classification of the sample, if possible. @p NaN
   * otherwise.
   */
  virtual double learnOne(double label, double weight);

  /**
   * Reads a feature vector from the @p features input and stores it
   * into a batch of samples that will be used as the training samples
   * when the training thread is started. This function is called by
   * #process() during learning if the learning algorithm is not
   * capable of on-line learning or if batch-based learning is
   * requested by the user. Subclasses should respect the value of
   * #learningBatchSize. If the current number of samples in the batch
   * is larger than #learningBatchSize (and the batch size is not -1),
   * the sample must be either be discarded or one of the older
   * samples must be replaced, depending on #fullBufferBehavior.
   *
   * @param label the classification of the training sample, or @p NaN
   * if not applicable.
   *
   * @param weight the importance of the sample. If the @p weight
   * input is not connected, this value will always be 1.0. Learning
   * algorithms that are not capable of weighted learning will ignore
   * this value.
   */
  virtual void collectSample(double label, double weight) = 0;

  /**
   * Called when the operation stops after on-line learning. 
   * Subclasses can override this function to clean up the resources
   * allocated by on-line learning.
   */
  virtual void finishOnlineLearning();

  /**
   * Sets the learning error message.
   */
  void setLearningError(const QString& learningError);
  
  /**
   * With supervised learning algorithms, this function reads the @p
   * label input and returns the class label. With non-supervised
   * learning algorithms, @p NaN will be returned.
   *
   * @throw PiiExecutionException& if the input object cannot be
   * converted to a @p double.
   */
  double readLabel() const;

  /**
   * Returns the value read from the @p weight input, or 1.0 if the
   * input is not connected.
   *
   * @throw PiiExecutionException& if the input object cannot be
   * converted to a @p double.
   */
  double readWeight() const;

  /**
   * Get a pointer to the input that receives feature vectors.
   */
  PiiInputSocket* featureInput();

  /**
   * Get a pointer to the (optional) input that receives class indices
   * for the incoming feature vectors.
   */
  PiiInputSocket* labelInput();
    
  /**
   * Get a pointer to the (optional) input that receives weights
   * associated with incoming samples.
   */
  PiiInputSocket* weightInput();
  
  /**
   * Get a pointer to the ouput that emits a class index for each
   * incoming feature vector (also in learning mode).
   */
  PiiOutputSocket* classificationOutput();

  /**
   * Reads a feature vector from the @p features input and calls
   * algorithm.learnOne() using @a label as the class label and @a
   * weight as the importance.
   *
   * @exception PiiExecutionException& if the features are of
   * incorrect type or size.
   */
  template <class SampleSet> int learnOne(PiiLearningAlgorithm<SampleSet>& algorithm,
                                          double label,
                                          double weight = 1.0);
  /**
   * Reads a feature vector from the @p features input and calls
   * classifier.classify() using it as the input.
   */
  template <class SampleSet> double classify(PiiClassifier<SampleSet>& classifier);

  void setProgressStep(double progressStep);
  double progressStep() const;
  void setLearningBatchSize(int learningBatchSize);
  int learningBatchSize() const;
  void setFullBufferBehavior(PiiClassification::FullBufferBehavior fullBufferBehavior);
  PiiClassification::FullBufferBehavior fullBufferBehavior() const;

  void aboutToChangeState(State newState);

private:
  QMutex* learningMutex();
  void init();
  bool learningThread();
  bool startLearningThread(bool startThread);
};


template <class SampleSet>
bool PiiClassifierOperation::learnBatch(PiiLearningAlgorithm<SampleSet>& algorithm,
                                        const SampleSet& samples,
                                        const QVector<double>& labels,
                                        const QVector<double>& weights)
{
  algorithm.setController(this);
  
  try
    {
      algorithm.learn(samples, labels, weights);
    }
  catch (PiiClassificationException& ex)
    {
      setLearningError(ex.message());
      return false;
    }
  
  return true;
}

template <class SampleSet>
int PiiClassifierOperation::learnOne(PiiLearningAlgorithm<SampleSet>& algorithm,
                                     double label,
                                     double weight)

{
  PiiClassification::FeatureReader<SampleSet> readFeatures;
  int iFeatures = featureCount();
  typename PiiClassification::FeatureReader<SampleSet>::ConstFeatureIterator features =
    readFeatures(featureInput(), &iFeatures);
  double dClassification = algorithm.learnOne(features, iFeatures, label, weight);
  return int(dClassification);
}

template <class SampleSet>
double PiiClassifierOperation::classify(PiiClassifier<SampleSet>& classifier)
{
  PiiClassification::FeatureReader<SampleSet> readFeatures;
  int iFeatures = featureCount();
  return classifier.classify(readFeatures(featureInput(), &iFeatures));
}

#endif //_PIICLASSIFIEROPERATION_H
