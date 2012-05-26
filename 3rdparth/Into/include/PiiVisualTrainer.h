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

#ifndef _PIIVISUALTRAINER_H
#define _PIIVISUALTRAINER_H

#include <PiiDefaultOperation.h>
#include <QLinkedList>
#include <QSize>
#include <QVector>
#include <QTime>
#include <PiiQImage.h>
#include "PiiVisualTrainerInputSocket.h"

class QImage;

/**
 * Visual trainer is a component that is used in assigning class
 * labels for unknown data. The operation is configured with a
 * visualization-based user interface in which the user is able to
 * select class labels with the mouse. The most typical use of a
 * visual trainer is in assigning labels to images whose feature
 * vectors are first vector-quantized with the SOM.
 *
 * The @p configurator UI of a visual trainer provides a view in which
 * a large image is shown together with a set of small images
 * extracted from it. The small images are shown on a two-dimensional
 * map.
 *
 * @image html visualtrainer.png
 *
 * The PiiVisualTrainerOperation works by converting an incoming @p
 * class @p index into a user-assigned class label or many different
 * labels. The labels are configured with @p layers. A layer defines
 * the labels a user is able to select for the data points in the
 * visualizer. Currently, there are three types of layers: floating
 * point, class labels, and boolean. With a floating point layer, the
 * user can label each data point with a floating point value (min/max
 * and resolution are user-determinable). Floating point labeling is
 * useful, for example, in process measurement applications, or in
 * visual inspection applications where there is no clear difference
 * between "good" and "bad" material. If a zero label denotes good and
 * one denotes bad, 0.55 might mean "slightly worse than average". A
 * class label layer makes it possible to select class labels from a
 * prebuilt set of names. Such labeling is most commonly used in
 * defect detection and recognition applications. A boolean layer is
 * no more than a two-label class layer.
 *
 * @par Inputs
 *
 * @in image - a large image (any image type)
 *
 * @in filename - the name of the large image. This name is used as
 * the icon title in node browser and as the image title. (QString,
 * optional)
 *
 * @in subimage - sub-images extracted from the large image (any
 * image type)
 *
 * @in location - the location of the sub-image within the large image
 * (1-by-4 PiiMatrix<int> or @p PiiIntRectangle)
 *
 * @in class index - the index of the code vector (e.g. in a
 * SOM) of the sub-image's feature vector. This determines where on
 * the map the sub-image ends up. The default map (grid) size is
 * 10-by-10. In this case, class index 0 means the upper left corner,
 * 10 means the first column of the second row and so on. (int)
 *
 * @in labelX - an optional label for the input sample. X is a
 * zero-based layer index. In supervised training, the labels of the
 * samples are known in advance. %PiiVisualTrainer can make use of
 * this information in automatically labeling the samples. For
 * class/boolean layers, the most likely class is selected by voting
 * among the input labels. For float layers, the average of input
 * values will be used.
 *
 * The @p class @p index input must always be connected while the
 * others are optional. If @p subimage and @p location are connected,
 * also @p image must be connected, but @p subimage and @p location
 * can also be left unconnected.
 *
 * @p Image and @p filename are always grouped together. If @p
 * filename is connected, it must always have an object whenever @p
 * image does. @p Subimage and @p location are also grouped, but they
 * accept objects with a higher flow level. @p Class @p index is
 * grouped to @p subimage and @p location if they are connected,
 * otherwise it will be grouped to @p image.
 *
 * If @p subimage and @p location are unconnected, no separate image
 * display for the a large image will be shown in the configurator. 
 * Instead, the input images all end up into the visual selector.
 *
 * @par Outputs
 *
 * @out labelX - an user-assigned label (double) for the incoming
 * class index. If the operation is not configured, zero (0) is
 * emitted for each incoming class index. X denotes the zero-based
 * index of the layer. For example, if there is only one layer, @p
 * label0 emits its label. The label can be any floating point value,
 * but its value range and resolution are limited by the layer type. 
 * For class label layers, emitted values are cardinal numbers.
 *
 * All outputs are synchronized to the @p subimage input, if it is
 * connected. Otherwise, the outputs will be synchronized to @p image.
 *
 * @ingroup PiiClassification
 */
class PiiVisualTrainer : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * This property determines the buffer mode for the visual trainer. 
   * The enum type @p BufferMode determines two possible values: @p
   * BufferTotal, which is the default value, and @p BufferPerCell. In
   * the mode @p TotalBuffer there is one buffer per visual trainer.
   * Once the buffer gets full, the image that came first to the visual
   * trainer, will be destroyed. In the mode @p PerCell, there is own
   * buffer for each cell in the grid. Once the buffer in a cell gets
   * full, the last image that has been assigned to the cell, will be
   * destroyed. The buffer size for both modes is determined with the
   * property "bufferSize". If the buffer size is set to non-positive
   * value (including zero), all the sub-images belonging to a single
   * large image are buffered, regardless of the current buffer mode. 
   * In this case it is recommened to use the buffer mode TotalBuffer,
   * because it works in a more efficient way.
   *
   * This property also has an effect on how #bufferFrequency works. 
   * Refer to the documentation of the #bufferFrequency property for
   * more information.
   */  
  Q_PROPERTY(BufferMode bufferMode READ bufferMode WRITE setBufferMode);

  Q_ENUMS(BufferMode);
  
  /**
   * The number of sub-images the visual trainer will buffer. The
   * exect meaning of this differs a little bit, depeding which is the
   * current buffer mode. Refer to the documentation of the property
   * "bufferMode" to get more information of that. The default value is 0.
   */
  Q_PROPERTY(int bufferSize READ bufferSize WRITE setBufferSize);

  /**
   * Defines the maximum frequency of images added to the image
   * buffer. For example if the value is set to 10, only ten pictures
   * in a second are added into the buffer, and if the value is set to
   * 0.1, an image is added to the buffer at most every 10 seconds. 
   * The exact behavior of this property varies depending on the value
   * of the #bufferMode property. In mode @p BufferTotal, there is a
   * single frequency counter for all cells, and the time when images
   * has been added in one cell, affects to the decision, whether
   * images are added in to the other cells. In mode @p BufferPerCell
   * there is own frequency counter for each cell, and a desicion
   * whether an image is added into the buffer of a cell or not, is
   * done independently of the fact when images has been previously
   * stored in the other cells. With value 0, all images arrived will
   * be stored in the buffer. The default value is 0.
   */
  Q_PROPERTY(double bufferFrequency READ bufferFrequency WRITE setBufferFrequency);
  
  /**
   * The size of the selection grid. The default value is 10 x 10
   * squares.
   */
  Q_PROPERTY(QSize gridSize READ gridSize WRITE setGridSize);

  /**
   * A rather complex property that defines selector layers. For each
   * entry in this list, a new labeling layer is created. The
   * configuration of the layer is stored as a map of name-value
   * pairs. That is, each QVariant in this list is actually a
   * QMap<QString, QVariant>. The configuration parameters depend on
   * the type of the layer:
   *
   * @par All layers
   *
   * @li @p type - layer type, currently either "float" or "class"
   * (QString)
   *
   * @li @p name - the user-visible name of the layer (QString)
   *
   * @li @p opacity - strength of overlay coloring (int, 0 =
   * transparent, 255 = opaque)
   *
   * @li @p disabled - is layer disabled? (bool, default: false)
   *
   *
   * @par Floating point layers
   *
   * @li @p min - minimum value for a label (double, default: 0)
   *
   * @li @p max - maximum value for a label (double, default: 1)
   *
   * @li @p resolution - value resolution (double, default: 0.1)
   *
   * @li @p startColor - gradient start color (QColor, default:
   * transparent red)
   *
   * @li @p endColor - gradient end color (QColor, default: opaque
   * red)
   *
   *
   * @par Class label layers
   *
   * @li @p classNames - class names (QStringList, default: "Good",
   * "Defect")
   *
   * @li @p classColors - color codes (QList<QVariant>, QVariants must
   * be QColors, default: arbitrary colors)
   *
   *
   * @code
   * QMap<QString, QVariant> firstLayer;
   * firstLayer["type"] = "float";
   * firstLayer["name"] = "Float layer";
   * firstLayer["max"] = 2;
   * firstLayer["endColor"] = QColor(Qt::blue); //Note: Qt::blue is an int
   *
   * QMap<QString, QVariant> secondLayer;
   * secondLayer["type"] = "class";
   * secondLayer["name"] = "Class name layer";
   * secondLayer["classNames"] = QStringList() << "Good" << "Fair" << "Poor";
   * secondLayer["classColors"] = QVariantList() << QColor(Qt::green) << QColor(Qt::yellow) << QColor(Qt::red);
   *
   * trainer->setProperty("layers", QList<QVariant>() << firstLayer << secondLayer);
   * @endcode
   */
  Q_PROPERTY(QVariantList layers READ layers WRITE setLayers);

  /**
   * Enable or disable image display. If display is enabled (the
   * default), incoming images will be sent to all open UI components,
   * if such exist. Setting this value to @p false causes the
   * operation to stop sending images. This property is useful if one
   * needs to keep the @p configurator UI open but there is no need to
   * fill it with images.
   */
  Q_PROPERTY(bool displayEnabled READ displayEnabled WRITE setDisplayEnabled);
  
  /**
   * A read-only property that tells the current number of labeling
   * layers.
   */
  Q_PROPERTY(int layerCount READ layerCount);

  /**
   * A read-only property that lists the user-visible names for
   * layers.
   */
  Q_PROPERTY(QStringList layerNames READ layerNames);

  /**
   * A read-only property that lists the names for enabled layers.
   */
  Q_PROPERTY(QStringList enabledLayerNames READ enabledLayerNames);
  
  /**
   * The current set of user-assigned labels for the selection grid. 
   * The list contains a list of labels for each layer. Thus its
   * length equals layer count (not output count if layers are
   * disabled!). labels[0] is a QVariantList holding the labels for
   * layer 0 and so on. Each label list has an entry for each cell in
   * the selection grid. labels[0][0] is the first label and so on.
   */
  Q_PROPERTY(QVariantList labels READ labels WRITE setLabels);

  /**
   * The currently suggested set of labels. This property is
   * meaningful only if the label inputs are connected and labels have
   * been assigned to incoming samples. The visual trainer tries to
   * find the most likely value for each label given the training
   * labels. The format of the returned list is analogous to #labels.
   */
  Q_PROPERTY(QVariantList suggestedLabels READ suggestedLabels);

  PII_OPERATION_SERIALIZATION_FUNCTION
public:

  /**
   * An enumeration used in determinining the buffer mode of the
   * PiiVisualTrainer operation. It is used as a parameter in the
   * #bufferMode property.
   *
   * @lip BufferTotal - one buffer is used for the whole visual
   * trainer.
   *
   * @lip BufferPerCell - each cell in the grid has its own buffer.
   */
  enum BufferMode { BufferTotal, BufferPerCell };

  PiiVisualTrainer();
 
  void setBufferMode(PiiVisualTrainer::BufferMode mode);
  PiiVisualTrainer::BufferMode bufferMode() const;

  void setBufferSize(int size);
  int bufferSize() const;

  void setBufferFrequency(double frequency);
  double bufferFrequency() const;
  
  void setGridSize(const QSize& size);
  QSize gridSize() const;

  bool displayEnabled() const;
  void setDisplayEnabled(bool enabled);

  void setLabelTable(const QVector<QVector<double> >& labels);
  QVector<QVector<double> > labelTable() const;

  void setLayers(const QList<QVariant>& layers);
  QList<QVariant> layers() const;

  int layerCount() const;
  QStringList layerNames() const;
  QList<int> layerIds() const;

  QStringList enabledLayerNames() const;

  void setLabels(const QVector<QVector<double> >& labels);
  void setLabels(const QVariantList& labels);
  QVariantList labels() const;

  QVariantList suggestedLabels() const;

  void check(bool reset);

  QVariant socketProperty(PiiAbstractSocket* socket, const char* name) const;

  PiiVisualTrainer* clone() const;

signals:
  void subImageAdded(QImagePtr image, double x, double y);
  void subImageRemoved(QImagePtr image);
  void mainImageChanged(QImagePtr image);
  void allSubImagesArrived();
  void allSubImagesRemoved();
  void gridSizeChanged(const QSize&);
  void layersChanged(const QVariantList&);
  void labelsChanged(const QVariantList&);

protected:
  void process();
  void syncEvent(SyncEvent* event);
  PiiFlowController* createFlowController();

public slots:
  void storeSubImage(QImagePtr image, int classIndex);
  void removeFromBuffer(int startFrom);

private:
  /**
   * Get a swap list depends on given variantLists. This function create
   * and return the list, which length is the same than old list.
   * The elements of the swap list tells where the old element is in
   * the new list. If the element doesn't contains in new list, the
   * swap list element == -1. For example:
   *
   * old list:  A  B  C  D  E  F  G
   *
   * new list:  C  D  G
   *
   * swap list:-1 -1  0  1 -1 -1  2
   */
  QList<int> swapList(const QVariantList& oldList, const QVariantList& newList);

  void updateInformation(QImagePtr image, const PiiMatrix<int>& location);
  void storeSubImage(const PiiVariant& bigImage, const PiiMatrix<int>& location, int classIndex);
  template <class T> QImage* subMatrix(const PiiVariant& bigImage, const PiiMatrix<int>& location);
  
  void clearBuffers();
  void setSizeOfBuffList();

  void createOutput(const QVariantMap& properties);
  void createInput(PiiVisualTrainerInputSocket::LayerType type);

  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    PiiVisualTrainer::BufferMode bufferMode;
    int iBufferSize;
    double dBufferFrequency;
    // The interval (in mill seconds), how often the buffer is updated.
    // The value is calculated from dBufferFrequency.
    int iBufferUpdateInterval;
    int iMainImageIndex;
    int iEnabledLayerSum;
    
    QSize gridSize;
    int iCellCount;
    bool bDisplayEnabled;
    
    QList<QVariant> lstLayers;
    QVector<QVector<double> > lstLabels;
    QVector<int> lstEnabledLayerIndex;
    
    PiiVariant varMainImage;
    // List of lists containing sub images
    QVector<QLinkedList<QImagePtr> > lstBuffers;
    // The following list is used in deciding, whether an image can be
    // added in the buffer.
    QVector<QTime> lstTimes;
    
    PiiInputSocket *pIndexInput, *pMainImageInput, *pSubImageInput, *pLocationInput, *pFileNameInput;
    
    QMutex configMutex;
    bool bMainImageConnected, bLocationConnected, bSubImageConnected, bFileNameConnected, bResetImageBuffers;
    
    QList<QPair<PiiMatrix<int>, int> > lstBufferedLocations;
  };
  PII_D_FUNC;
};
#endif
