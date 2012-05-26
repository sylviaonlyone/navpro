/* This file is part of Into.
 * Copyright (C) Intopii.
 * All rights reserved.
 */

#error "Documentation.h is not intended for inclusion. Its sole purpose is to provide documentation for a package."

/**
 * @mainpage Into
 * 
 * Into is a highly portable C++ programming environment for creating
 * machine intelligence pattern recognition, and image processing
 * applications.
 * 
 * Into performs tasks ranging from real-time image processing,
 * feature extraction and selection to classification and
 * visualization. It provides automatic parallelization and network
 * transparency.
 *
 * Applications built with Into are composed of a few specialized
 * components: an execution engine called @ref Ydin "Ydin", a number
 * of @e plugins (PiiPlugin) containing @e operations (derived from
 * PiiOperation), and a @e configuration that describes the
 * application. Configurations can be built programmatically or with a
 * visual development environment with no programming. This way, it is
 * possible to create efficient pattern recognition applications very
 * quickly.
 *
 * This documentation works both as an API reference and a programming
 * tutorial. It describes the internal structure of the platform and
 * guides one in creating applications with a few lines of C++.
 *
 * The documentation is organized as follows:
 *
 * @li @subpage architecture introduces the overall architecture of
 * the platform and explains the purposes of its components.
 *
 * @li @subpage creating_configuration goes through the necessary
 * steps of programming and building an application configuration and
 * illustrates the process with a simple example.
 *
 * @li @subpage saving_and_loading shows how to save and retrieve
 * configurations to and from files and other I/O devices.
 *
 * @li @subpage execution_details goes a little under the hood and
 * explains what goes on inside the execution engine.
 *
 * @li @subpage conventions is the definite style guide for Into code. 
 * If you intend to contribute to the platform, you need to read this.
 *
 * @li @subpage custom_operations shows how to create your own
 * operations, how to add sockets, handle different data types and
 * synchronization events etc. Read this and you are in business.
 *
 * @li @subpage compound_operations goes a bit further by combining
 * multiple operations into a compound.
 *
 * @li @subpage creating_plugins gives instructions to create your own
 * plug-ins.
 *
 * @li @subpage serialization_mainpage describes the internal
 * structure of the marshalling/demarshalling API and its use.
 *
 * @li @subpage additional_documentation_pages contains miscellaneous
 * documentation of specific parts of the Into library.
 *
 * @li @subpage building_instructions describes how to configure and
 * build the platform itself.
 *
 * @li @subpage third_party_components describes the way third party
 * components are included into the platform.
 */

/**
 * @page additional_documentation_pages Additional Documentation
 *
 * @li @subpage calibration_overview
 * @li @subpage calibration_stereo
 */

/**
 * @page architecture Architecture
 *
 * The figure below shows a conceptual overview of the Into platform. 
 * The platform consists of an execution engine and a number of
 * different operations organized into plug-in modules. The dashed
 * line surrounds a set of image analysis operations that make up a
 * plug-in. An application, surrounded by the dotted circle, is built
 * by a configuration that makes use of operations in different
 * plug-ins.
 *
 * @image html pinta_ydin_devenv.png
 *
 * The Development Environment is a visual tool for building and
 * testing configurations. It provides a convenient graphical user
 * interface in which a user can create new configurationss by simply
 * dragging and dropping operations. Ydin works under the hood and
 * makes it possible to visually evaluate the functioning of a
 * configuration.
 *
 * The Development Environment saves the configurations and their
 * visual representation separately so that the configuration can be
 * processed without the development environment.
 *
 * @section Software Components
 *
 * @image html architecture_components.png
 *
 * The figure above shows a diagram of the essential components of
 * Into and their interdependencies. The core of the platform, called
 * Ydin, consists of an execution engine (PiiEngine), interfaces to
 * executable operations (PiiOperation), interfaces and classes for
 * handling plug-ins (PiiPlugin), and a resource database
 * (PiiResourceDatabase).
 *
 * With no plug-ins loaded, Ydin is unable to perform any useful
 * tasks. In addition to the plug-ins, a configuration that describes
 * the operations, their configuration and connections, is needed. The
 * configuration can be created either programmatically or with a
 * graphical Development Environment.
 *
 * Operations are software components that receive some input data and
 * produce some results after processing. An operation may also have
 * no inputs or no ouputs, in which case it is considered a producer
 * or a consumer, respectively. Each operation typically runs in a
 * separate execution thread, receiving objects and emitting new ones.
 *
 * The operations can work either synchronously or asynchronously. For
 * example, an image source can emit images in its own internal
 * frequency, irrespective of any consumers of the image data
 * (asynchronous operation). The receivers process the data if they
 * aren't too busy doing other things. In most cases, however, it is
 * necessary to process all images captured. In this case the image
 * source halts until the next processing layer is done with
 * processing before it sends the next image.
 *
 * Each operation can have any number (0-N) of input and outputs
 * called sockets (PiiSocket). Each socket has a data type or multiple
 * data types associated with it. That is, each socket either reads or
 * emits a limited set of object types types of object. Input and
 * output sockets with matching types can be connected to each other,
 * and an output can be connected to multiple inputs. Note, however,
 * that the platform does not prevent you from connecting non-matching
 * sockets.
 *
 * The figure below shows two operations connected to each other. The
 * first one, PiiLbpOperation reads in images and outputs features. 
 * The second one, @ref PiiSomOperation "PiiSomOperation<float>",
 * reads in features and outputs a code vector index (classification). 
 * The feature output of the LBP operation is connected to the feature
 * input of the SOM.
 *
 * @image html architecture_lbp_som.png
 *
 * A @ref PiiPlugin "plug-in" is a bundle of operations dynamically
 * loadable into PiiEngine. When loaded, a plug-in registers its
 * operations to the resource database so that each operation can be
 * dynamically created at run time. Plug-ins can also provide
 * extensions to the Development Environment and register custom data
 * types and classes.
 *
 * @section Design Principles
 *
 * The most important design guidelines in Into are:
 *
 * @li @b Performance. Application software created with Into is ready
 * to be used in demanding applications such as industrial visual
 * inspection. Processing data happens asynchronously, and in
 * parallel. The platform can automatically utilize all the processors
 * of a computer without additional programming effort. Even in a
 * single-processor machine, the multi-threaded architecture allows
 * the processor to utilize its full capacity. Asynchronous operation
 * means that incoming data is processed as soon as it appears,
 * minimizing unnecessary waiting.
 *
 * @li @b Re-use. A generic input/output interface between software
 * components allows all components to interact with each other
 * without translations. All new operations have all the old ones at
 * their disposal. Standardized data formats ensure that all
 * operations speak the same language.
 *
 * @li @b Extensibility. Operations are loaded into Into via a plug-in
 * system that makes it possible to adjust the capabilities of Into
 * and its memory requirements at run time. Custom operations and
 * plug-ins can be created and used in a straightforward manner.
 *
 * @li <b>Easy programming</b>. Often, an intelligent analysis
 * application can be created with just a few lines of code. One only
 * needs to learn a couple of functions for loading plug-ins, creating
 * operations, and for connecting operations to each other. It is also
 * possible to implement a fully-featured user interface, for which
 * the operations provide standard building blocks like configuration
 * dialogs.
 *
 * @li <b>Dynamic configuration</b>. Creating an application with Into
 * does not require programming. It is possible to create a
 * configuration with a graphical tool that stores the operations,
 * their properties, and connections into a file. A dynamically
 * configurable pattern recognition application reads the file,
 * creates the configuration at run-time and executes it. Not a single
 * line of code is needed. Applications created in this way perform
 * equivalently to their hard-coded counterparts; there is no
 * performance penalty in executing dynamic configurations.
 *
 * @li @b Portability. The main hardware platform for Into is the PC. 
 * Prebuilt binaries are currently available for Linux, OS X and
 * Microsoft Windows. Due to the requirements on portability,
 * performance and easy programming (no kidding), C++ is the
 * programming language of choice. No other programming language
 * combines productivity and speed better. (Let the flame war begin.) 
 * Into is built on top of a cross-platform C++ toolkit called %Qt
 * (see http://www.trolltech.com).
 */

/**
 * @page creating_configuration Creating a Configuration
 *
 * @section creating_steps Steps
 *
 * @b 1. The necessary first step in creating a new application
 * configuration is the creation of an instance of PiiEngine. 
 * The configuration is created by adding operations into the engine. 
 *
 * @code
 * PiiEngine engine;
 * @endcode
 *
 * @b 2. Depending on what operations are needed for the
 * configuration, the necessary plug-ins need to be loaded. Each
 * plug-in provides a set of operations that are available to the
 * engine upon successful loading of the plug-in. If the configuration
 * consists only of custom operations built into the application, this
 * step may be omitted.
 *
 * @code
 * engine.loadPlugin("piiimage");
 * @endcode
 * 
 * @b 3. Once the necessary plug-ins are loaded, operations are
 * created and added to the configuration. The operations may also
 * need to be configured. The configuration can be done manually or
 * with a user interface (see @ref creating_ui). In manual
 * configuration, the parameters are set with the @ref
 * PiiOperation::setProperty() "setProperty()" function.
 *
 * @code
 * PiiOperation* reader = engine.createOperation("PiiImageFileReader");
 * PiiOperation* writer = engine.createOperation("PiiImageFileWriter");
 * reader->setProperty("fileNamePattern", "images/*.jpg");
 * writer->setProperty("outputDirectory", "output");
 * writer->setProperty("extension", "jpg");
 * @endcode
 *
 * @b 4. The configuration is finished by connecting operations to
 * each other. There are multiple ways of doing this. The simplest one
 * is to use the @ref PiiOperation::connectOutput() "connectOutput()"
 * function. Remember that each socket has a unique name in the scope
 * of the enclosing operation. Thus, input and output sockets are
 * denoted in a generic manner by their textual names.
 "
 * @code
 * reader->connectOutput("image", writer, "image");
 * @endcode
 *
 * @b 5. The configuration is ready when all connections have been
 * created. The final step is to start the engine. Since the ideology
 * of the whole platform is in parallel execution, the execution of
 * the engine is also done in parallel. Therefore, one must make sure
 * the process that starts the engine does not exit before the engine
 * has done its job. A simple way of ensuring this is to use the @ref
 * PiiOperationCompound::wait() "wait()" function.
 *
 * @code
 * engine.execute();
 * engine.wait(PiiOperation::Stopped);
 * @endcode
 *
 * Put together, the little example above turns out to be a program
 * that reads all Jpeg files in a directory called "images" and writes
 * them into a directory called "output". This simple configuration is
 * shown in the figure below. Due to the default configuration of
 * PiiImageFileWriter, the output images are numbered sequentially
 * starting from zero. The name of the first output image will be
 * "img000000.jpg".
 *
 * @image html creating_reader_writer.png
 *
 * @section creating_ui User Interfaces
 *
 * UI architecture has completely changed in Into 2. To be rewritten.
 *
 * @section simple_example A Simple Example
 *
 * The following simple example will introduce an application that
 * reads images from files, thresholds them using an adaptive
 * thresholding technique, and writes the resulting binary images into
 * files. The program displays buttons for browsing the input images
 * and an image display that shows the result of thresholding.
 *
 * The program consists of a single file called @p Threshold.cc. It
 * reads the image files given as command-line arguments, sends them
 * to an image display and a thresholding operation, and finally
 * writes the thresholded images to the current working directory. The
 * configuration is described graphically below. Note that the
 * output images are stored as gray levels: 0 is black and 1 is white.
 * In most image viewers, the images will look pitch black.
 *
 * @image html creating_simple_example.png
 *
 * @include Threshold.cc
 *
 * Since Into is built on top of %Qt, compiling an Into application is
 * most convenient with %Qt's tools. One only needs to create a
 * project file that contains the instructions for a compiler. A
 * program called qmake converts the project file into
 * compiler-dependent instructions (a makefile). The project file is
 * called @p threshold.pro, and its contents are as follows:
 *
 * @code
 * LIBS += -L$$INTODIR/lib
 * INCLUDEPATH += $$INTODIR/include
 * unix|macx: LIBS += -lpiicore -lpiiydin -lpiigui
 * win32: LIBS += -lpiicore2 -lpiiydin2 -lpiigui2
 * SOURCES = Threshold.cc
 * TARGET = threshold
 * CONFIG += console
 * @endcode
 *
 * Replace @p into_install_dir with the directory the Into programming
 * environment is installed to. In the command prompt, simply type
 * qmake. This produces instructions (makefile) to compile the
 * application with a make tool. Then, build the application with
 * make. The process is slightly different with different compilers.
 *
 * @subsection creating_building_gcc Linux/OS X, GCC
 *
 * Open a terminal and type the following commands:
 *
 * @code
 * cd into_install_dir/doc/examples/threshold
 * qmake
 * make
 * ./threshold
 * @endcode
 *
 * @subsection creating_building_mingw Windows, MinGW
 *
 * Open the Qt command prompt and type the following commands:
 *
 * @code
 * cd into_install_dir\doc\examples\threshold
 * qmake
 * mingw32-make
 * release\threshold.exe
 * @endcode
 *
 * @subsection creating_building_msvc Windows, MSVC
 *
 * Open the Qt command prompt and type the following commands:
 *
 * @code
 * cd into_install_dir\doc\examples\threshold
 * qmake
 * nmake
 * release\threshold.exe
 * @endcode
 */

/**
 * @page saving_and_loading Saving and Loading
 *
 * Saving and loading configurations is straightforward. All one needs
 * to do is to invoke the @e serialization of a PiiEngine instance. 
 * Serialization is a process in which the members of a class instance
 * are recursively marhalled and written to an I/O device. In Ydin,
 * serialization happens through @e archives, objects that write and
 * read primitive data types in some specific format. Currently, there
 * are two actual implementations, one for plain text and another for
 * binary data. The choice between these formats is left to the user. 
 * Binary format is faster to read and write, but it is not as
 * portable as plain text, and non-readable to a human. Plain text is
 * not very readable, either, but can be edited with a text editor. It
 * is a bit slower to read and write, but often results in smaller
 * files.
 *
 * To save a configuration, one needs to open an I/O device, create an
 * archive that writes data to the device and write the data. Here is
 * how:
 *
 * @code
 * QFile file("file.cft");
 * file.open(QIODevice::WriteOnly);
 * PiiGenericTextOutputArchive ar(&file);
 * ar << engine;
 * @endcode
 *
 * It is possible to store any serializable data in a similar manner. 
 * See documentation @ref Serialization "chapter on serialiation" for
 * a thorough explanation. Reading a configuration is just as simple:
 *
 * @code
 * QFile file("file.cft");
 * file.open(QIODevice::ReadOnly);
 * PiiGenericTextInputArchive ar(&file);
 * ar >> engine;
 * @endcode
 *
 * Any data written into an archive can be read back in the same order
 * it was written.
 *
 * All bundled operations can be saved and restored with the shown
 * technique. If you need to serialize your custom operations, they
 * need to be made serializable. Placing them into a plug-in is the
 * easiest way (see @ref creating_plugins), but making them
 * serializable "manually" is not hard either (see @ref
 * Serialization).
 */

/**
 * @page execution_details Execution
 *
 * @section how_ydin_executes How Ydin Executes Operations
 * 
 * PiiEngine is completely unaware of any connections between the
 * operations that have been added to it with @ref
 * PiiEngine::addOperation() "addOperation()". When a configuration is
 * executed, PiiEngine first checks whether the operations are ready
 * to be executed by calling @ref PiiOperation::check() of each
 * operation it currently holds. If everything is fine, PiiEngine
 * calls @ref PiiOperation::execute() of all the operations. It does
 * not know what happens, but is able to track the state of each
 * operation in the configuration. The engine stops running once all
 * of its operations have stopped either spontaneously, due to an
 * error or because of an explicit @ref PiiEngine::interrupt()
 * "interrupt".
 * 
 * The configuration may stop either spontaneously or after a user
 * command. Spontaneous exit may happen, for example, if an image
 * source runs out of images. In such a case the image source sends a
 * signal that tells the next processing layer to also stop once they
 * have finished processing the last image. The signal is transmitted
 * to all connected operations, which causes the engine to finally
 * stop. Now, if the configuration has multiple processing pipelines
 * that are not connected to each other by forward paths, i.e. there
 * are multiple independent producers, the engine will not stop until
 * all producers have finished their execution.
 * 
 * A working configuration must have at least one producer operation. 
 * Emission of objects from this operation causes processing in
 * the others. When a producer emits an object through its output,
 * other operations connected to that output are signalled to start a
 * new processing round. Depending of the type of the receiving
 * operation, the processing may happen in parallel or in the same
 * execution thread with the sender. Parallel execution can happen in
 * any of the computer's processors, depending on the load of the
 * system.
 *
 *
 * @section synchronization Synchronization
 *
 * Although there is no clock signal that would synchronize
 * operations, all operations still work in sync with each other by
 * default. This is ensured by making each output socket send one
 * object for each one read from an input. Thus, in the case of
 * multiple input sockets, the operation waits until an object is
 * present in each and sends one and only one object to each output.
 * 
 * This method works as far as only one object is sent for each input
 * object. Things get complicated when an operation changes the rate
 * of objects by emitting either more or less objects. This is still
 * acceptable if the flows with different object rates are handled in
 * separate pipelines. Problems appear when these pipelines are
 * connected. One perfectly valid solution is to not allow such
 * connections. The engine itself does not care about synchronization,
 * and you are free to implement the operations just as you like. If
 * you can remain asynchronous, you are probably faster than
 * synchronous. In many cases, however, the problem needs to be faced
 * an solved. The following sections describe the default
 * synchronization mechanism that is supported by all built-in
 * operations. You are free not to follow the flock and implement your
 * own system, but be aware of compatibility issues.
 *
 * @subsection synchronization_default Default Synchronization Mechanism
 * 
 * @image html synchronization.png
 * 
 * A typical synchronization issue appears when a large image is split
 * into smaller sub-images, and an operation later in the processing
 * pathway needs to merge the sub-images originating from a single
 * large image, as shown in the figure above. Given no synchronization
 * information, the merging operation (PiiImagePieceJoiner) has no way
 * of knowing how many @p locations and @p labels are received for
 * every @p image.
 * 
 * The problem could be solved by forcing all operations to emit
 * exactly one object for each object they read. In such a scenario,
 * the sub-images should be packed into an object that represents
 * multiple images. The drawback is that all operations should be
 * aware of a new data type: a list of images. Furthermore, they
 * should also know a list of lists, a list of lists of lists etc. An
 * even more severe drawback would be that all the sub-images should
 * be processed in a batch, and one could not utilize parallel
 * computation in an efficient manner.
 * 
 * The solution adopted in Ydin is that each operation indicates the
 * input socket (if any) an output socket it is synchronized to. For
 * example, the outputs of an @ref PiiImageSplitter "image splitter"
 * are synchronized to its inputs because image pieces will only be
 * emitted if an image is received in the @p image input. Given this
 * information, Ydin is able to detect, for example, when all the
 * sub-images originating from a single large image have been received
 * by an operation. Another requirement is that an operation that
 * breaks the "normal" 1:1 input-output relationship on synchronized
 * sockets informs the system whenever it misbehaves. Please refer to
 * @ref configuring_synchronization for detailed instructions.
 *
 * @subsection synchronization_example Synchronization Example
 *
 * Let us go through a short example to illustrate how the
 * synchronization technique really works. In the figure above,
 * PiiImageFileReader emits an image through its @p image output
 * whenever it has decoded a file. The image goes to the @p image
 * input of PiiImagePieceJoiner. In PiiImagePieceJoiner, @p label and
 * @p location inputs are grouped and form a synchronized socket
 * group. The @p image input has no synchronized pair. Therefore, the
 * operation does not need to wait for more objects but can process
 * the image immediately.
 *
 * The image also goes to the @p image input of PiiImageSplitter. 
 * Since the operation has no other inputs, it is processed right
 * away. Before the operation actually sends something to its outputs,
 * it calls PiiOutputSocket::startMany() to indicate that it may send
 * more than one object. This causes a synchronization tag to be
 * emitted through the @p subimage and @p location outputs. It is
 * subsequently received by @p CustomClassifier, which has only one
 * input. The tag will be accepted and passed to the @p label output.
 *
 * Now there is a synchronization tag in the @p label input of
 * PiiImagePieceJoiner. Since @p label and @p location are in the same
 * synchronization group, the operation needs to wait. But not too
 * long, because a similar tag was just sent from the @p location
 * output of PiiImageSplitter. Depending on how @p CustomClassifier is
 * implemented it may happen that the tag appears first in the @p
 * location input of PiiImagePieceJoiner, but this does not matter. 
 * Once both are there, the operation passes the tag to all outputs
 * synchronized to that group of inputs. It also marks the @e flow @e
 * level of the inputs to be one higher than before and sends a @ref
 * PiiFlowProcessor::Listener::syncEvent() "synchronization event".
 *
 * Now that the synchronization tags are processed PiiImageSplitter
 * starts sending actual objects. @p CustomClassifier will be
 * processed right after receiving an image, and PiiImagePieceJoiner
 * every time both @p label and @p location are there. Finally,
 * PiiImageSplitter stops splitting the image and calls
 * PiiOutputSocket::endMany(). This will send another synchronization
 * tag, which tells the operations later in the pipeline to lower
 * their flow level. PiiImagePieceJoiner notices that the flow level
 * of the @p label and @p location inputs is now equal to that of the
 * @p image input and sends another synchronization event.
 *
 * Meanwhile, PiiImageFileReader has been busy decoding a new image. 
 * It may have already completed its job and sent the image to
 * PiiImageSplitter and PiiImagePieceJoiner. It may even be fetching
 * the third image, but it cannot send it because the inputs have not
 * been cleared yet. Anyhow, the image is there again, or at least it
 * will be there in a while, and the process starts from the
 * beginning.
 *
 * @subsection synchronization_stop_pause Stopping and Pausing
 *
 * The execution of an engine can stop spontaneously. In the example
 * above, PiiImageFileReader sends a @e stop @e tag when it runs out
 * of images. Whenever an operation receives such a tag in all of its
 * inputs, it will finish its execution. If the configuration has many
 * independent producers, all of them must stop before the whole
 * engine changes its state to @p Stopped.
 *
 * @ref PiiOperation::pause() "Pausing" and @ref PiiOperation::stop()
 * "non-spontaneous stopping" work in a similar manner, but they are
 * initiated outside of the operation. When an operation with no
 * connected inputs receives a pause or stop signal, it will turn into
 * @p Paused or @p Stopped state whenever it is done with its current
 * processing round. It subsequently send a @e pause @e tag or a @p
 * stop @p tag which, when received into all inputs of an operation,
 * turns that operation into @p Paused or @p Stopped state. This will
 * finally pause or stop the whole processing pipeline.
 */

/**
 * @page custom_operations Custom Operations
 *
 * Custom operations are needed when the set of prebuilt operations is
 * not enough for solving an inspection problem. The need for custom
 * operations also arises when one needs to connect an inspection
 * application to process control or to custom hardware.
 * 
 * @section operation_interfaces Operation Interfaces
 * 
 * When PiiEngine executes a configuration, it only needs a way of
 * starting, stopping and pausing operations. To create a
 * configuration, one also needs a way of retrieving the inputs and
 * outputs of an operation. Finally, a way of creating
 * operation-specific user interfaces is needed. These are the
 * functions all Ydin-compatible operations must implement. The
 * functions are defined in the PiiOperation class, which must be
 * inherited when creating a custom operation.
 * 
 * Although it is possible to create a fully custom operation from
 * scratch, it is almost always better to use a partial implementation
 * of the PiiOperation interface called PiiDefaultOperation. This
 * class takes care of synchronization, and is derived from
 * PiiBasicOperation that handles the adding and removing of sockets. 
 * This section concentrates on implementing operations based on
 * PiiDefaultOperation. PiiOperation and PiiBasicOperation can be used
 * in some special situations, but they require more work for to
 * properly cope with synchronization, for example.
 * 
 * PiiDefaultOperation uses a @ref PiiDefaultOperation::FlowController
 * "flow controller" to control the flow of objects through the
 * operation. Whenever the controller decides that it is time to
 * process, a @ref PiiDefaultOperation::OperationProcessor "processor"
 * invokes a protected @ref PiiDefaultOperation::process() "process"
 * function. This function is overridden by subclasses to perform the
 * actual processing.
 *
 * PiiDefaultOperation provides two processing modes: @e threaded and
 * @e simple. The processing mode can be changed with the
 * @ref PiiDefaultOperation::processingMode property.
 *
 * In simple mode, @p process() will be called immediately at the
 * reception of a new input object. There is no separate thread. It
 * logically follows that simple mode cannot be used with operations
 * that just produce data without an external trigger. Such operations
 * must be processed in parallel. Simple mode is the right choice when
 * the overhead in parallel processing exceeds the complexity of the
 * operation. Such operations are, for example, simple logical and
 * arithmetic calculations.
 * 
 * In threaded mode, a separate processing thread is started for the
 * operation. The thread sleeps when no data is available in inputs
 * and is awakened by each incoming object. Threaded operations are
 * used when the the actual processing takes a relatively long time.
 *
 * A typical declaration for an operation is as follows:
 *
 * @code
 * class MyOperation : public PiiDefaultOperation
 * {
 * public:
 *   MyOperation();
 * protected:
 *   virtual void process();
 * };
 * @endcode
 * 
 * The constructor should add the necessary input and output sockets
 * to the operation. The @p process function reads all objects from
 * input sockets and processes them. It should then send the results
 * of processing to output sockets, or to any other interface such
 * as a user interface, network socket or database.
 * 
 * @section input_and_output_sockets Input and Output Sockets
 * 
 * Connections between operations are handled with objects called
 * sockets. PiiSocket is a common super-interface for both input and
 * output sockets, and it is derived by PiiInputSocket and
 * PiiOutputSocket. All sockets have a unique name in the scope of the
 * enclosing operation, which makes it possible to refer to them by
 * name. They also have a number of dynamic properties that are used
 * by the Development Environment.
 * 
 * Let us assume MyOperation has one input and one output. The task
 * of the operation is simply to pass an input object to the output. The
 * sockets are created as shown below:
 * 
 * @code
 * MyOperation::MyOperation()
 * {
 *   // Create an input socket
 *   addSocket(new PiiInputSocket("input"));
 *   // Create an output socket
 *   addSocket(new PiiOutputSocket("output"));
 * }
 * @endcode
 * 
 * Since we actually need no processing, the @p process function is a
 * one-liner:
 * 
 * @code
 * void MyOperation::process()
 * {
 *   emitObject(readInput());
 * }
 * @endcode
 * 
 * The function reads an incoming object (@ref
 * PiiInputSocket::objectAt()) and emits it through the output socket
 * (@ref PiiOutputSocket::emitObject()). This is all one needs to do
 * for a functional operation.
 * 
 * @section data_types Data Types
 * 
 * @subsection data_types_variants Variants
 * 
 * The data types passed between operations are encapsulated in a
 * class called PiiVariant. PiiVariant holds a pointer (@p void*) to
 * any object and an ID number for the type of the object. Operations
 * determine the type of the received object by inspecting the ID
 * number. There is a large number of prebuilt data types (see the API
 * documents for the PiiYdin namespace and PiiVariant class), and
 * nothing prevents one from creating new ones as far as unique ID
 * numbers are selected.
 * 
 * Instead of copying an object each time it is passed from an
 * operation to another, Ydin uses implicit reference counting and a
 * primitive but efficient garbage collector.
 * 
 * Let us assume for a while that our illustrative @p MyOperation not
 * only passes the object but also increments it if it happens to be
 * an integer. The process function must now inspect the type of the
 * incoming object:
 * 
 * @code
 * void MyOperation::process()
 * {
 *   PiiVariant obj = readInput();
 *   if (obj.type() == PiiVariant::IntType)
 *     emitObject(obj.valueAs<int>() + 1);
 *   else
 *     emitObject(obj);
 * }
 * @endcode
 * 
 * The @ref PiiVariant::type() function returns the type ID of a
 * variant object. The @p if clause in the example tests if the type
 * ID equals the type ID of an integer. If that is the case we know
 * the value of the variant can be safely retrieved as an @p int. Note
 * that instead of modifying the incoming object a new object is
 * created that holds the value of the old one plus one. It is
 * possible to modify the value in place, but this practise is not
 * encouraged because the object may be simultaneously used by any
 * number of operations. In-place modifications can be justified by
 * performance reasons in some rare cases, and only if the
 * configuration is created with static program code and one knows for
 * sure that only one operation will use the object.
 * 
 * The @ref PiiVariant::valueAs() function template returns a
 * reference to the held object as the template type, in this case @p
 * int. @ref PiiOutputSocket::emitObject() is also a template that
 * supports any data type. The function template creates a new
 * PiiVariant out of the value passed as the only argument. It gets
 * the type ID for the new variant by another function template, @ref
 * Pii::typeId(). It has specializations for all the types defined in
 * PiiVariant.h and PiiYdinTypes.h. (Please remember to include
 * PiiYdinTypes.h if you use types other than the primitive ones.)
 * 
 * Variants can also be created excplicitly. This way it is possible
 * to give a custom type ID to an object.
 * 
 * @code
 * PiiVariant newObj(obj.valueAs<int>()+1, PiiVariant::IntType);
 * emitObject(newObj);
 * @endcode
 *
 * This technique is useful if you want to mark a derived object with
 * the type ID of a parent that is more widely supported. Another case
 * is if you want to use an existing data type in a special role that
 * needs to be handled separately.
 * 
 * @subsection common_data_types Common Data Types
 * 
 * In Into, the most common form of data that is passed between
 * operations are numbers and matrices. Matrices are also used to
 * represent images, which might first feel a bit awkward. (Matlab
 * users should feel home.) All primitive data types are reflected by
 * corresponding type IDs in the PiiVariant class itself. Matrices,
 * complex numbers and colors are defined in the PiiBaseType
 * namespace.
 * 
 * The Into system uses a class template called PiiMatrix in
 * representing all matrix and image types. For example. 
 * PiiMatrix<unsigned char> represents 8-bit gray-scale images. A
 * color is also denoted by a class template, called PiiColor or
 * PiiColor4, depending on whether the color has three or four
 * channels. An RGB image with eight bits per color channel is thus
 * represented by PiiMatrix<PiiColor<unsigned char> >.
 * 
 * Four channel colors are used to optimize speed. Since four bytes
 * equals 32 bits, addressing such a color always happens at a
 * word-aligned memory address. The drawback is that more memory is
 * needed. Four channel colors are, however, in intense use because
 * image I/O is performed with 32-bit colors.
 * 
 * All image operations in Into are assumed to support at least the
 * following image types:
 * 
 * <table>
 * <tr><th>Image type</th><th>Class</th></tr>
 * <tr><td>8-bit gray</td><td>%PiiMatrix\<unsigned char\> </td></tr>
 * <tr><td>16-bit gray</td><td>%PiiMatrix\<unsigned short\> </td></tr>
 * <tr><td>32-bit gray</td><td>%PiiMatrix\<unsigned int\> </td></tr>
 * <tr><td>floating point gray</td><td>%PiiMatrix\<float\></td></tr>
 * <tr><td>24-bit color</td><td>%PiiMatrix\<%PiiColor\<unsigned char\> \></td></tr>
 * <tr><td>32-bit color</td><td>%PiiMatrix\<%PiiColor4\<unsigned char\> \></td></tr>
 * <tr><td>48-bit color</td><td>%PiiMatrix\<%PiiColor\<unsigned short\> \></td></tr>
 * <tr><td>floating point color</td><td>%PiiMatrix\<%PiiColor\<float\> \></td></tr>
 * </table>
 * 
 * All matrix types have a corresponding type ID in the PiiYdin
 * namespace. For example, PiiMatrix<unsigned char> is reflected by
 * PiiYdin::UnsignedCharMatrixType.
 * 
 * Matrices are used not only for images but also for areas, points,
 * feature vectors etc. Please see the documentation for PiiYdin.h and
 * PiiPoint for more information.
 * 
 * @subsection supporting_multipe_types Supporting Multiple Types
 * 
 * Let us now assume that MyOperation should add one to all pixels of
 * each image it reads in. Since it is assumed to support all image
 * types, there should be a routine to perform the addition for all
 * the image types listed above. Doing this manually would be
 * error-prone, laborious and stupid:
 * 
 * @code
 * void MyOperation::process()
 * {
 *   PiiVariant obj = readInput();
 *   using namespace PiiYdin;
 *   switch (obj.type())
 *   {
 *   case UnsignedCharMatrixType:
 *     addOneTo8BitGray(obj.valueAs<PiiMatrix<unsigned char> >);
 *     break;
 *   case UnsignedCharColor4Type:
 *     addOneTo32BitColor(obj.valueAs<PiiMatrix<PiiColor4<unsigned char> > >());
 *     break;
 *   // etc...
 *   }
 * }
 * @endcode
 * 
 * The suggested way is to define a function template that handles
 * this all. We'll first need to add this function to the class
 * declaration:
 * 
 * @code
 * class MyOperation : public PiiDefaultOperation
 * {
 * ...
 * private:
 *   template <class T> addOne(const PiiVariant& obj);
 * };
 * @endcode
 * 
 * The file PiiBaseType.h contains useful macros for building the
 * necessary switch clauses. Since we are supporting all image types,
 * we need a macro called PII_ALL_IMAGE_CASES:
 * 
 * @code
 * void MyOperation::process()
 * {
 *   PiiVariant obj = readInput();
 *   switch (obj->type())
 *     {
 *       PII_ALL_IMAGE_CASES(addOne, obj);
 *     default:
 *       PII_THROW_UNKNOWN_TYPE(inputAt(0));
 *     }
 * }
 * @endcode
 * 
 * The result is that the function template function @p addOne() is
 * instantiated with a different template parameter depending on the
 * type ID of the incoming object. If the type ID does not match any
 * of our supported types, the @ref PII_THROW_UNKNOWN_TYPE macro
 * throws an exception that PiiEngine handles as an error.
 * 
 * In the implementation of the function template something like the
 * following needs to be done:
 * 
 * @code
 * template <class T> MyOperation::addOne(const PiiVariant& obj)
 * {
 *   const PiiMatrix<T> &image = obj.valueAs<PiiMatrix<T> >();
 *   emitObject(image + T(1));
 * }
 * @endcode
 * 
 * The first line takes the value of the passed object as a matrix of
 * any type. The correct template type @p T is ensured by the @ref
 * PII_ALL_IMAGE_CASES macro. The second line adds one to the matrix
 * and emits the result through the only output socket. Using @p T(1)
 * ensures that the operation also works with colors.
 * 
 * Since matrices are the most common form of data passed between
 * operations, their use should be as easy as possible. For this
 * reason, PiiBaseType.h provides a lot more similar convenience
 * macros. The variations are explained in the API documentation.
 *
 * Let us still work through another example. This version does the
 * same as the previous one, but converts color images to gray scale.
 * For this, we need a new conversion function:
 *
 * @code
 * class MyOperation : public PiiDefaultOperation
 * {
 * ...
 * private:
 *   template <class T> convertAndAdd(const PiiVariant& obj);
 * };
 * @endcode
 * 
 * The process function is modified like this:
 * 
 * @code
 * void MyOperation::process()
 * {
 *   PiiVariant obj = readObject();
 *   switch (obj.type())
 *     {
 *       // Gray images work with addOne
 *       PII_GRAY_IMAGE_CASES(addOne, obj);
 *       // Color images are converted with convertAndAdd
 *       PII_COLOR_IMAGE_CASES(convertAndAdd, obj);
 *     default:
 *       PII_THROW_UNKNOWN_TYPE(inputAt(0));
 *     }
 * }
 * @endcode
 * 
 * Within the switch block, color and gray-scale images are separated
 * into two different processing functions with two marcos. The old
 * one (@p addOne()) is now used for gray-scale images only. The color
 * version looks like this:
 * 
 * @code
 * template <class T> MyOperation::convertAndAdd(const PiiVariant& obj)
 * {
 *   // T is a PiiColor or PiiColor4 now.
 *   // A handy typedef for the color's actual element type.
 *   typedef typename T::Type Element;
 *   PiiMatrix<T> &image = obj.valueAs<PiiMatrix<T> >();
 *   emitObject(static_cast<PiiMatrix<Element> >(image) + 1);
 * }
 * @endcode
 * 
 * The first line of the function defines a @p typedef for the
 * primitive type that forms the color's channels. Converting the
 * color image to gray scale is straightforward: just perform a type
 * cast to the type of the color channels. The third line does just
 * that, adds one to the result, and emits it through the output
 * socket.
 * 
 * @section configuring_synchronization Configuring Synchronization
 *
 * Synchronizing ansychronous processing units is a subtle issue and
 * needs to be carefully considered when implementing custom
 * operations. Fortunately, most operations work in one-to-one
 * input-output correspondence: every incoming object is processed and
 * one result object is sent. For other types of processing, the
 * programmer must be aware of synchronization.
 *
 * An operation that produces more than one result per input object
 * must inform the corresponding output socket that it is going to
 * break the 1:1 rule. A good example is PiiImageSplitter that emits
 * 1-N subimages for each incoming image.
 *
 * In the following example, the operation produces two outputs for
 * each input.
 * 
 * @code
 * void MyOperation::process()
 * {
 *   PiiVariant obj = readInput();
 *   outputAt(0)->startMany();
 *   outputAt(0)->emitObject(obj);
 *   outputAt(0)->emitObject(obj);
 *   outputAt(0)->endMany();
 * }
 * @endcode
 * 
 * Before the program starts sending the objects, it tells the output
 * socket that multiple objects will follow instead of the expected
 * single one. This is accomplished with @ref
 * PiiOutputSocket::startMany(). When the operation is done sending,
 * it informs the socket with @ref PiiOutputSocket::endMany(). The
 * same process applies to situations where the operation omits
 * sending an object. In such a case it only signals @p startMany and
 * @p endMany without any @p emitObject calls in between. Thus, the
 * meaning of "many" is actually "other than one".
 * 
 * The internal effect of this in Ydin is that the <em>flow level</em>
 * of the output socket is first raised and then lowered. Ydin tracks
 * the flow level of synchronized sockets through the whole pipeline
 * of operations, which is why one needs to indicate what sockets are
 * synchronized to each other. Input sockets within an operation are
 * synchronized when their flow levels are equal.
 * 
 * Omitting the @p startMany and @p endMany signals causes no problems
 * if outputs with different object rates are not connected to the
 * inputs of a single operation. However, breaking this or any other
 * synchronization rule causes hard-to-debug problems in complex
 * configurations.
 * 
 * @subsection sync_groups Synchronization Groups
 *
 * Input and output sockets are arranged into synchronization groups. 
 * Sockets with the same group id always work in sync with each other:
 * there must be an object in all grouped input sockets before
 * processing can be performed. And the results must be emitted to all
 * output sockets with the same group id.
 *
 * By default, all sockets belong to group 0. This means that all
 * inputs must be filled before processing will happen and that each
 * output will always emit an object whenever the processing is
 * performed.
 *
 * PiiDefaultOperation uses a PiiDefaultFlowController to handle
 * synchronization in operations with more than one input group. 
 * PiiDefaultFlowController can be configured to handle parent-child
 * relationships between input groups. In PiiImagePieceJoiner, for
 * example, the @p image input has group id 0 (parent), and it expects
 * to receive a large image. The @p location and @p label inputs have
 * group id 1 (child), and they expect to receive objects at a higher
 * flow level. Since the flow level of the @p location and @p label
 * inputs is higher than that of the @p image, a synchronization error
 * would occur if the inputs were connected the other way round.
 *
 * If a strict parent-child relationship the parent group will be
 * processed first. Operations should however prefer loose
 * relationships because they are more efficient and are less prone to
 * deadlocks. In some cases (such as PiiObjectReplicator) this is
 * unfortunately not possible.
 *
 * The @ref PiiDefaultOperation::process() "process()" function is
 * called whenever a group of sockets needs to be processed. The
 * function must process one and only one group at a time. To find out
 * which group is active, use the @ref
 * PiiDefaultOperation::activeInputGroup() function.
 *
 * To group sockets into synchronization groups, use the @ref
 * PiiSocket::setGroupId() function.
 * 
 * @subsection capturing_sync_events Capturing Synchronization Events
 * 
 * Most operations don't need to care about synchronizing sockets with
 * different object rates because they read one object from each input
 * socket on each processing round. There are however situations where
 * the object rate in one input socket is greater than that of the
 * other.
 * 
 * PiiDefaultOperation has a virtual function called @ref
 * PiiDefaultOperation::syncEvent() "syncEvent()" that needs to be
 * overridden if one needs to capture synchronization events. This
 * function is called just before an object is about to be processed
 * in a group or any of its child groups, and whenever all input
 * sockets are back on the same flow level.
 *
 * Let us assume our custom operation has two input sockets: one that
 * receives an image (@p _pImageInput) and another that receives any
 * number of interesting objects (e.g. their coordinates) within the
 * image. The @p process() function is used to handle the objects, but
 * there you have no clue when the last location related to the large
 * image was received. This is where the @p syncEvent() function is
 * used.
 *
 * @code
 * MyOperation::MyOperation()
 * {
 *   // Default group id is 0
 *   addSocket(_pImageInput = new PiiInputSocket("image"));
 *   addSocket(_pLocationInput = new PiiInputSocket("location"));
 *   _pLocationInput->setGroupId(1);
 * }
 *
 * void MyOperation::process()
 * {
 *   if (activeInputGroup() == 0) // large image received
 *     doSomething();
 *   else // location received
 *     doSomethingElse();
 * }
 *
 * void MyOperation::synEvent(SyncEvent* event)
 * {
 *   // See if inputs are in sync with the image input.
 *   // If we have just two sync groups, this check is actually unnecessary.
 *   if (event->type() == SyncEvent::EndInput &&
 *       event->groupId() == _pImageInput->groupId())
 *     {
 *       // Do something here
 *       // PiiImagePieceJoiner, for example, would
 *       // perform the actual joining algorithm now.
 *     }
 * }
 * @endcode
 * 
 * @section configuration_interface Configuration Interface
 * 
 * Since all operations eventually inherit from QObject, QObject's
 * property system is used in configuring the operations. The details
 * of the system are best described in %Qt's documentation. It suffices
 * here to show an example. We will modify our MyOperation so that
 * instead of incrementing bypassing integers by one it adds a
 * user-configurable value to it. The class declaration must be
 * changed as follows:
 * 
 * @code
 * class MyOperation : public PiiDefaultOperation
 * {
 *   Q_OBJECT
 * 
 *   Q_PROPERTY(int change READ change WRITE setChange);
 * public:
 *   MyOperation();
 * 
 *   int change() const { return _iChange; }
 *   void setChange(int change) { _iChange = change; }
 * 
 * protected:
 *   virtual void process();
 * 
 * private:
 *   int _iChange;
 * 
 * };
 * @endcode
 * 
 * The Q_OBJECT macro is a Qt feature that needs to be included in all
 * classes using properties or the signal and slot system. We will
 * omit the details here. The next line in the class declaration tells
 * that the operation has a @p change property whose type is an @p
 * int. The value of the property is retrieved with a function called
 * @p change(). A new value is set with @p setChange(). The
 * implementations of these functions retrieve and store an internal
 * variable called @p _iChange. Now, the value of this property can be
 * set with <tt>setProperty("change", value)</tt>, where @p value is
 * an integer.
 * 
 * The final step is to modify the process function to make use of the
 * property:
 * 
 * @code
 * void MyOperation::process()
 * {
 *   PiiVariant obj = readInput();
 *   if (obj.type() == PiiVariant::IntType)
 *     emitObject(obj.valueAs<int>() + _iChange);
 *   else
 *     emitObject(obj);
 * }
 * @endcode
 * 
 * 
 * @section de_interface Interfacing with the Development Environment
 * 
 * This section is still to be written.
 * 
 * 
 * @section using_custom_operation Using Custom Operation in Configuration
 * 
 * If custom operations are used in a program whose configuration is
 * created programmatically (non-dynamically), it is not necessary to
 * create a dynamically loadable plug-in that exports the operation to
 * the resource database. Instead, the code can be compiled directly
 * into the application.
 * 
 * A common pitfall in creating custom operations is that their
 * declaration is not handled by @p moc, the Qt's meta object
 * compiler. The @p moc is needed in converting the properties,
 * signals and slots to C++ code. The declarations of all classes with
 * the Q_OBJECT macro must reside in files mentioned in the HEADERS
 * section of the project file. Let us assume that we placed the
 * declaration of class @p MyOperation into @p MyOperation.h. Let us
 * further assume that we need @p MyOperation in the thresholding
 * program created in @ref simple_example. To make @p moc process our
 * new operation, @p threshold.pro needs to be modified by adding the new
 * source and header files:
 * 
 * @code
 * SOURCES = Threshold.cc MyOperation.cc
 * HEADERS = MyOperation.h
 * @endcode
 * 
 * Adding the operation to the configuration is straighforward. It
 * will be inserted between PiiThresholdingOperation and
 * PiiImageFileWriter like this:
 * 
 * @code
 * PiiOperation* my = new MyOperation;
 * my->setProperty("change", -6);
 * engine.addOperation(my);
 * threshold->connectOutput("image", my, "input");
 * my->connectOutput("output", writer, "image");
 * @endcode
 * 
 * Note that since the operation is not registered to the resource
 * database, it can not be created by its name with @ref
 * PiiOperationCompound::createOperation(). Instead, an instance of
 * the operation is created and connected to PiiEngine with @ref
 * PiiOperationCompound::addOperation().
 * 
 * In this example, the @p change property is set to -6. Since the
 * operation will never see other objects than images, the value will
 * affect nothing. But the example illustrates how to set your own
 * properties.
 */

/**
 * @page compound_operations Compound Operations
 *
 * Compound operations are composed of sub-operations that together
 * form a logical processing unit. An example of such an operation is
 * PiiVisualSomClassifier that combines a PiiSomOperation and a
 * PiiVisualTrainer to a visually trainable SOM classifier. Compound
 * operations are derived from PiiOperationCompound, and PiiEngine is
 * in fact just a compound operation.
 *
 * @section compound_inputs_and_outputs Inputs and Outputs
 *
 * The input and output sockets in a compound can be either @e aliased
 * or @e proxied. An alias is more efficient as it just reveals a
 * socket that is actually in one of the sub-operations. If you delete
 * either the sub-operation or the socket, the alias disappears.
 *
 * Using a proxy (PiiSocketProxy) has the advantage that it won't
 * disappear unless explicitly told so. An alias will be destroyed if
 * the operation containing the aliased socket is destroyed. This will
 * obviously break all connections to the socket. Furthermore, a proxy
 * input can be connected to many input sockets within the compound. 
 * This is not possible with an alias. The downside is that passing
 * objects through a proxy is slightly slower than through an alias.
 *
 * @section compound_accessing_components Accessing Components within a Compound
 *
 * All operations added as childs to a compound with the @ref
 * PiiOperationCompound::addOperation() "addOperation()" function will
 * be placed under the compound in QObject's object hierachy. That is,
 * the @ref QObject::parent() "parent()" function of the child
 * operation will return a pointer to the compound. Therefore,
 * QObject::findChild() can be used to find child operations by name. 
 * All direct child operations can be retrieved with
 * PiiOperationCompound::childOperations(). In documentation, child
 * operations are listed by their @p objectName under a section titled
 * "Children".
 *
 * It is a common task to assign properties to the child operations of
 * a compound. To help this, the non-virtual member function
 * QObject::setProperty() is overridden by
 * PiiOperation::setProperty(). The property() function is treated
 * similarly. Although this breaks C++ conventions, it makes it easy
 * to set properties to child objects. Assuming a compound has a child
 * operation with an object name "foo" that has a property called
 * "bar" one can access the property directly through the compound by
 * calling <tt>compound->property("foo.bar")</tt>. The nesting can, of
 * course, be deeper if needed. "foo.baz.bar" is a valid property name
 * if "foo" is a compound that contains "baz". Care must however be
 * taken that the operations are accessed through PiiOperation
 * pointers, not QObject pointers. The same approach is used when
 * accessing sockets within the compound. See PiiOperationCompound for
 * examples.
 *
 * @section compound_serialization Serializing a Compound
 *
 * PENDING
 *
 * @section compound_example Compound Example
 *
 * PENDING
 */

/**
 * @page creating_plugins Creating Plug-ins
 *
 * Plug-ins are dynamic parts of the Into platform that can be loaded
 * and unloaded on request. Placing operations into plug-ins has the
 * advantage that they don't need to be linked to the application
 * itself. The approach also forces one to use a generic interface for
 * all operations, which is good (TM) because it helps one in
 * resisting the temptation of including one gazillion interface
 * definitions to the main executable. It is also possible to unload
 * the features that are no longer needed by an application. Granted,
 * the generic interface makes it hard to work with complex data
 * types, but it is still worth the trouble.
 *
 * @section creating_plugins_bones Bare Bones
 *
 * All Ydin-compatible plug-ins must contain two query functions the
 * system can find with dlsym(): @p pii_get_plugin_name() and @p
 * pii_get_plugin_version(). Both must return a <tt>const char*</tt>. 
 * In addition, plug-in libraries may contain static data that needs
 * to be registered to the @ref PiiYdin::resourceDatabase() "resource
 * database".
 *
 * @ref PiiPlugin.h contains a bunch of useful macros for dealing with
 * plug-ins. 
 *
 * @par Implementing a plug-in
 *
 * Implementing a plug-in is easy. The only thing one needs to do is
 * to a) select a unique name for the plug-in and b) use the @ref
 * PII_IMPLEMENT_PLUGIN macro in a .cc file:
 *
 * @code
 * PII_IMPLEMENT_PLUGIN(MyPlugin);
 * @endcode
 *
 * @section creating_plugins_exporting Exporting Classes
 *
 * @par Registering Operations
 *
 * Once the plug-in has been implemented, it is time to register your
 * operations to it. Use the @ref PII_REGISTER_OPERATION macro:
 *
 * @code
 * #include "MyOperation.h"
 *
 * PII_IMPLEMENT_PLUGIN(MyPlugin);
 *
 * PII_EXPORT_OPERATION(MyOperation);
 * @endcode
 *
 * You can repeat the macro as many times as needed. The macro
 * registers the named class to resource database. Now that your class
 * is registered to the plug-in, it can be created dynamically by its
 * name. Use the PiiYdin::createResource() function for this.
 *
 * Note that the operations you register this way must be @e
 * serializable. (See @ref Serialization.)
 *
 * @par Registering Other Classes
 *
 * Plugins are allowed to register not just types derived from
 * PiiOperation or QObject. For this, the @ref PII_REGISTER_CLASS
 * macro is provided. Note that registering dynamically instantiable
 * classes is useful only if the type of the class is known to those
 * creating the instances. Usually, the superclass of a registered
 * class needs to be defined outside of the plug-in.
 *
 * @code
 * // Register a class derived from PiiCameraDriver
 * PII_EXPORT_CLASS(MyCameraDriver, PiiCameraDriver);
 * @endcode
 *
 * @section building_plugins Building
 *
 * Once the plugin is ready, it must be compiled and linked. All the
 * necessary options are provided by piiplugin.pri, located in the
 * plugins directory. Minimally, all one needs in a project file is
 * this (example taken from the @p piibase plug-in):
 *
 * @code
 * PLUGIN = Base
 * include (../piiplugin.pri)
 * @endcode
 *
 * This assumes that the plug-in is placed under the plugins
 * directory. Then type @p qmake followed by @p make.
 *
 * If a plug-in depends on other plug-ins, one needs to tell the
 * compiler and the linker where to find the necessary headers an
 * libraries, including those required by the other plug-in. To avoid
 * this hassle, the build system can resolve recursive dependencies
 * for you. Assume your plug-in depends on @p piidsp and @p piicolors
 * plug-ins. Add the dependencies to a file called @p dependencies.pri
 * in the project folder:
 *
 * @code
 * DEPENDENCIES = Dsp Colors
 * @endcode
 *
 * PENDING This may be obsolete now.
 *
 * MSVC needs special linker instructions to force linkage even when
 * there are no obvious connections between the two plug-ins. For this
 * to work, not just the plug-in library name but also the name of the
 * plug-in class must be known. Therefore, the names must be given
 * exactly as shown in the example. The syntax is case-sensitive. The
 * @p plugindeps.pri project include file assumes that plug-in class,
 * library, and path names follow strict conventions. That is, if you
 * place @p FooBar in @p DEPENDENCIES, the plug-in must reside in a
 * directory called @p foobar/. The name of the library must be @p
 * libfoobar.so or @p foobar.dll. If your plug-in doesn't follow these
 * conventions, you are at your own. But there is really no magic
 * behind. Just inspect @p plugindeps.pri and find out what to do with
 * your own naming conventions.
 */

/**
 * @page building_instructions Building
 * 
 * PENDING
 */

/**
 * @page third_party_components Third Party Components
 *
 * PENDING
 */
 
