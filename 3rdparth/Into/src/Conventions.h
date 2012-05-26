/* This file is part of Into.
 * Copyright (C) Intopii.
 * All rights reserved.
 */

#error "Conventions.h is not intended for inclusion. Its sole purpose is to provide documentation for a package."

/**
 * @page conventions Coding Conventions
 *
 * The purpose of this document is to achieve the impossible: make the
 * work of a team of programmers look uniform. We are not going into
 * details on why standardization is important or why we selected this
 * stupid convention over the one you like. The discussion would be
 * endless. All code in Into must adhere to the guidelines given here. 
 * No excuses will be accepted.
 * 
 * The document is divided into a the following subsections:
 *
 * @subpage conventions_indentation declares the indentation holy war
 * over.
 *
 * @subpage conventions_naming defines the naming style in Into code.
 *
 * @subpage conventions_bincomp gives an overview on how to write good
 * libary code and to make code binary-compatible.
 *
 * @subpage conventions_templates tells more than an image. Use these
 * and we don't need to explain header guards, copyright statements
 * etc.
 *
 * @subpage conventions_documentation defines the style of formal and
 * informal documentation.
 *
 * @subpage conventions_miscellaneous lists miscellaneous conventions
 * that don't belong to anywhere else.
 */

/**
 * @page conventions_bincomp Library code and binary compatibility
 *
 * For an overview of good library programming practices please
 * familiarize yourself with the following:
 *
 * @li <a href="http://techbase.kde.org/Policies/Library_Code_Policy">KDE
 * library code policy</a>.
 *
 * @li <a href="http://techbase.kde.org/Policies/Binary_Compatibility_Issues_With_C%2B%2B">Binary
 * Compatibility Issues With C++</a>
 *
 * Although Into doesn't exactly follow the KDE policy, the documents
 * give important insight on the matter.
 *
 * D pointers in Into are called simply "d" just like in KDE. An
 * important difference is that the "private" class (in Into we speak
 * about a data class) is usually an inner class:
 *
 * @code
 * class MY_EXPORT MyBaseClass
 * {
 * public:
 *   MyBaseClass();
 *   virtual ~MyBaseClass();
 *
 *   int intMember() const;
 *
 * protected:
 *   class Data
 *   {
 *   public:
 *     Data();
 *     virtual ~Data();
 *     int iMember;
 *   } *d;
 *   MyBaseClass(Data* data);
 * };
 *
 * // In .cc
 * MyBaseClass::Data::Data() : iMember(0) {}
 * MyBaseClass::Data::~Data() {}
 *
 * MyBaseClass::MyBaseClass() : d(new Data) {}
 * MyBaseClass::MyBaseClass(Data* data) : d(data) {}
 * MyBaseClass::~MyBaseClass() { delete d; }
 *
 * int MyBaseClass::intMember() const { return d->iMember; }
 * @endcode
 *
 * A derived class inherits the inner data class of its parent class. 
 * Therefore, the both the class and its data class must have a
 * virtual destructor. The getter function @p intMember() refers to
 * the data class directly by its name, @p d.
 *
 * @code
 * class MY_EXPORT MyDerivedClass
 * {
 * public:
 *   MyDerivedClass();
 *   ~MyDerivedClass();
 *   double doubleMember() const;
 *   void calculate();
 *
 * private:
 *   class Data : public MyBaseClass::Data
 *   {
 *   public:
 *     Data();
 *     double dMember;
 *   };
 *   PII_D_FUNC;
 * };
 *
 * // In .cc
 * MyDerivedClass::Data::Data() : dMember(0) {}
 * MyDerivedClass::MyDerivedClass() : MyBaseClass(new Data) {}
 * MyDerivedClass::~MyDerivedClass() {}
 *
 * double MyDerivedClass::doubleMember() const { return _d()->dMember; }
 * void MyDerivedClass::calculate()
 * {
 *   PII_D;
 *   d->dMember *= 1.1;
 * }
 * @endcode
 *
 * Since the base class is responsible for deleting the data class, @p
 * MyDerivedClass doesn't do it in its destructor. It should however
 * have one to retain binary compatibility in case you later need it.
 *
 * The @p doubleMember() member function cannot directly access @p
 * d->dMember because the type of @p d is @p MyBaseClass::Data, which
 * only has @p iMember. Instead, it uses the @p _d() inline function
 * defined by the @p PII_D_FUNC macro. The function casts @p d to the
 * type of derived class' data, in this case @p MyBaseClass::Data.
 *
 * If a member function needs the @p d pointer many times, it is
 * faster to store the @p d pointer on the stack. To do this, place
 * the @p PII_D macro at the beginning of a function body. In a @p
 * const function, use <tt>const PII_D</tt>. The macro defines a local
 * variable called (surprise) @p d, which hides the class member.
 */

/**
 * @page conventions_indentation Indentation
 *
 * Indentation may be the most common flavor of holy wars when it
 * comes to programming style, and that is why we start with it. Since
 * the other holy war is over and the default indentation style in
 * Emacs is the <a
 * href="http://www.gnu.org/prep/standards/standards.html">GNU
 * style</a>, the winner of the indentation holy war is clear. Here's
 * what Into code looks like:
 *
 * @include IndentationDoc.cc
 *
 * Indentations are spaces, not tabs. And there are two of them. No
 * more, no less.
 * 
 * @note The GNU coding standard is followed only when it comes to
 * indentation. The GNU standard is for C, and Into is C++.
 */

/**
 * @page conventions_miscellaneous Miscellaneous Conventions
 * 
 * @li Function name is immediately followed by opening paren, which
 * is immediately followed by the first parameter. Parameters are
 * separated with a comma @e and a space.
 *
 * @code
 * functionCall(firstParam, secondParam);
 * @endcode
 *
 * @li Control statements (@p if, @p for, @p while, @p switch, @p
 * catch) and the opening paren are separated with a space. This
 * distinguishes them from function calls.
 * 
 * @code
 * if (_dSpeed == M_PI)
 *   doSomething();
 * @endcode
 *
 * @li Use spaces to separate statements in @p for clauses.
 *
 * @code
 * // i = 0, 1, 2
 * for (int i=0; i<3; ++i);
 * // i = 2, 1, 0
 * for (int i=3; i--; );
 * @endcode
 *
 * @li Favor pre-increment/decrement to post-increment/decrement.
 *
 * @code
 * while (true)
 *   {
 *      // Use this ...
 *      ++_iCounter;
 *      // ... instead of this
 *      _iCounter++;
 *   }
 * @endcode
 *
 * @li The @p const keyword is there to help you and the compiler. Use
 * it whenever possible. A member function that does not alter the
 * state of a class should always be @p const, even if it returned a
 * pointer or a reference to a member. A complex type as a function
 * parameter is usually passed as a @p const&.
 *
 * @li The order of class scopes is interface first. That is, the
 * first scope is public, then protected, then private. In all scopes
 * the order of declarations is types, member functions, attributes.
 *
 * @code
 * class MyClass
 * {
 * public:
 *   enum MyEnum { Value0, Value1 };
 *   void pubFunc();
 *
 * protected:
 *   void protFunc();
 *   int _iValue;
 *
 * private:
 *   typedef int Scalar;
 *   int privFunc(void);
 *   float _fValue;
 * };
 * @endcode
 *
 * @li Never use @p NULL. It is not C++. When you check for a null
 * pointer, compare to zero.
 *
 * @code
 * if (pData != NULL) {} // NO!
 * if (pData) {} // NO!
 * if (pData != 0) {} // Yes.
 * @endcode
 *
 * @li Throw and catch exceptions as references.
 *
 * @code
 * try
 *   {
 *     throw PiiException(tr("An error occured."));
 *     // Or better yet:
 *     PII_THROW(PiiException, tr("An error occured."));
 *   }
 * catch (PiiException& ex)
 *   {
 *   }
 * @endcode
 *
 * @li Leave at least one empty line between function/class
 * definitions.
 *
 * @code
 * // This is incorrect
 * void funcA()
 * {
 * }
 * void funcB()
 * {
 * }
 *
 * // This is correct
 * void funcC()
 * {
 * }
 *
 * void funcD()
 * {
 * }
 * @endcode
 *
 * @li We use the linefeed character (\\n) as a line terminator. 
 * Windows-style (\\r\\n) terminators are strictly prohibited.
 *
 * @li Try to keep lines shorter than 110 characters.
 */

/**
 * @page conventions_naming Naming
 *
 * <i>Much of the material in this section is taken from the "C++ Coding
 * Standard" text back in 2002 by Todd Hoff. Copyright (C) 1995-2002
 * Todd Hoff.</i>
 *
 * Names are the heart of programming. Only a programmer who understands
 * the system as a whole can create a name that "fits" with the system. 
 * If the name is appropriate everything fits together naturally,
 * relationships are clear, meaning is derivable, and reasoning from
 * common human expectations works as expected.
 *
 * Classes are often nouns. By making function names verbs and following
 * other naming conventions programs can be read more naturally.
 *
 * @li Name the class after what it is. If. you can't think of what it is
 * that is a clue you have not thought through the design well enough.
 *
 * @li Compound names of over three words are a clue your design may be
 * confusing various entities in your system. Revisit your design. Try a
 * CRC card session to see if your objects have more responsibilities
 * than they should.
 *
 * @li Avoid the temptation of bringing the name of the class a class
 * derives from into the derived class's name. A class should stand on
 * its own. It doesn't matter what it derives from. Generic suffices such
 * as "Driver" or "Model" are OK.
 *
 * @li Usually every function performs an action, so the name should
 * make clear what it does: checkForErrors() instead of errorCheck(),
 * dumpDataToFile() instead of dataFile().
 *
 * @li When confronted with a situation where you could use an all upper
 * case abbreviation instead use an initial upper case letter followed by
 * all lower case letters. No matter what. We use PiiRgbNormalizer
 * instead of PiiRGBNormalizer and PiiFft instead of PiiFFT.
 *
 * The Into style of naming types and functions closely follows the Java
 * standard, which is also similar to the %Qt style. In this chapter, the
 * following topics are considered:
 *
 * @li @ref conventions_classnames "Class Names"
 * @li @ref conventions_functionnames "Function Names"
 * @li @ref conventions_attrnames "Class Attribute Names"
 * @li @ref conventions_margnames "Function Parameters"
 * @li @ref conventions_type_prefixes "Type Prefixes"
 * @li @ref conventions_stacknames "Variable Names on the Stack"
 * @li @ref conventions_tnames "Type Names"
 * @li @ref conventions_mnames "\#defines and Macro Names"
 * @li @ref conventions_enames "Enumerations"
 * @li @ref conventions_tempnames "Templates"
 * @li @ref conventions_filenames "File Names"
 * @li @ref conventions_exceptions "Exceptions"
 *
 *
 * @anchor conventions_classnames
 * @par Class Names
 *
 *
 * @li Use upper case letters as word separators, lower case for the
 * rest of a word (e.g. PiiOperationCompound, PiiInputSocket).
 *
 * @li First character in a name is upper case
 *
 * @li Usually, there are no namespaces for classes. Instead, each class
 * name is preceded by "Pii". Namespaces are used in plug-ins (see
 * PiiImage, for example) to wrap functions that do not logically belong
 * to any class. If a class/struct is defined within a namespace, its
 * name must not be prepended with "Pii" (e.g. Pii::IsInteger). 
 *
 *
 * @anchor conventions_functionnames
 * @par Function Names
 *
 * @li Use the same rule as for class names, except for the first letter
 * which must be lower case. This rule makes it easy to distinguish
 * between function calls and constructor calls.
 *
 * @li Property getters and setters should be named according to Qt
 * style. That is, a property "fooBar" should have a getter called @p
 * fooBar() and a setter called @p setFooBar().
 *
 * @anchor conventions_type_prefixes
 * @par Type Prefixes
 *
 * @li Type prefixes should be used to indicate the type of variables,
 * both on the stack and as class attributes. The following is an
 * incomplete list of suitable prefixes:
 *
 * @lip i - @p int
 * @lip l - @p long
 * @lip d - @p double
 * @lip f - @p float
 * @lip str - QString
 * @lip lst - QList
 * @lip vec - QVector
 * @lip map - QMap
 * @lip mat - PiiMatrix
 * @lip clr - PiiColor
 * @lip p - pointer. Use this for real pointers and for classes that
 * masquerade as pointers, such as PiiSmartPtr.
 *
 * @li In some occasions, it is useful to be able to easily change a
 * type to another. For example, if there is no need to know that a
 * storage is a QVector instead of QList, @p lst could be used for
 * both.
 *
 * @li Do not exaggerate. A PiiMatrix<PiiColor<int> >* does not need
 * to be @p piclrmatImage. @p pColorImage is a better name.
 *
 *
 * @anchor conventions_attrnames
 * @par Class Attribute Names
 *
 * @li All private and protected attribute names must be prepended with
 * an underscore (_), followed by a type prefix.
 *
 * @li After the underscore and the type prefix, use the same rules as
 * for class names.
 *
 * Examples:
 *
 * @lip _iChildren - number of children, an @p int
 * @lip _lstChildren - the children, a QList, whose content type can be
 * anything. We won't go into extremes: template types can be left
 * unspecified.
 * @lip _strName - name, a QString
 * @lip _pData - pointer to some data
 * @lip _dSpeed - a double
 * @lip _fSpeed - a float
 *
 * @b Exception. The @e d pointer of a class should be called just @p
 * d, not @p _d.
 *
 *
 * @anchor conventions_margnames
 * @par Function Parameters
 *
 * @li The first character should be lower case.
 *
 * @li All word beginnings after the first letter should be upper case as
 * with class names.
 *
 * @li Function parameters do not need to be prefixed.
 *
 *
 * @anchor conventions_stacknames
 * @par Variable Names on the Stack
 *
 * @li Follow the same convention as with class attributes, just without
 * the initial underscore.
 *
 *
 * @anchor conventions_tnames
 * @par Type Names
 *
 * @li Use the same naming policy as for classes.
 *
 * Example: typedef PiiSharedPtr<MyObject> MyObjectPtr;
 *
 *
 *
 * @anchor conventions_mnames
 * @par \#defines and Macro Names
 *
 * @li Put \#defines and macros in all upper case using '_' as a word
 * separator. Macro parameters should follow the same convention.
 *
 * Example: @ref PII_REGISTER_OPERATION
 *
 *
 *
 * @anchor conventions_enames
 * @par Enumerations
 *
 * @li For both the type and the enumerated values use the same naming
 * policy as for classes.
 *
 * @li Repeat a keyword in all enumerated values. See @ref
 * Pii::PropertyFlag for an example.
 *
 * @li Prefer enums for boolean values in function parameters to make the
 * code more readable.
 *
 * @code
 * // Instead of this
 * void drive(bool careful);
 *
 * // Prefer this
 * enum DrivingMode { DriveCarefully, DriveCarelessly };
 * void drive(DrivingMode mode);
 *
 * // See the difference:
 * drive(true);
 * drive(DriveCarefully);
 * @endcode
 *
 *
 * @anchor conventions_tempnames
 * @par Templates
 *
 * @li Generic types with no special meaning are denoted with capital
 * letters. The convention is to name the first type @p T, the second
 * one @p U and so on, aplhabetically. If the types have similar
 * function, they can also be named @p T1, @p T2 etc.
 *
 * @li If a type has a clear role, the role should be reflected in the
 * type name, which should be formatted according to class name naming
 * conditions.
 *
 * @li The names of non-typename template parameters should follow
 * function name naming conditions.
 *
 * @li Leave a space after the @p template keyword.
 *
 * @code
 * template <class T> PiiMatrix;
 * template <bool condition, class Then, class Else> struct If;
 * @endcode
 *
 *
 * @anchor conventions_filenames
 * @par File Names
 *
 * @li Class @e declaration is always placed into a file whose name
 * equals the class name, and has a .h extension. The same applies to
 * namespaces. Examples: PiiOperation.h, PiiImage.h.
 *
 * @li Class definition is placed into a .cc file whose name matches the
 * header file. If the class is a template, the definition is placed into
 * a file that ends with -templates.h. Example: PiiLbp-templates.h. Both
 * .cc and -templates.h may be present.
 *
 *
 * @anchor conventions_exceptions
 * @par Exceptions
 *
 * @li It is a good idea to make iterators and function objects
 * compatible to stl. In these contexts, function and type names
 * should match stl, whose naming conventions are different. For
 * example, @ref Pii::UnaryFunction "adaptable unary function objects"
 * must contain @p typedefs for @p argument_type and @p result_type
 * (not @p ArgumentType and @p ResultType).
 *
 * @see PiiMatrix::begin()
 * @see PiiMatrix::constEnd()
 * @see Pii::Abs
 * @see Pii::Hypotenuse
 */

/**
 * @page conventions_templates Code Templates
 *
 * A template for header files. This file could contain a declaration
 * for MyClass. The name of the file is MyClass.h.
 *
 * @include MyClass.h
 *
 * A template for cc files. The name of the file is MyClass.cc.
 *
 * @include MyClass.cc
 *
 * If the class is a template class, its definition should be placed
 * into MyClass-templates.h:
 *
 * @include MyClass-templates.h
 */

/**
 * @page conventions_documentation Documentation Conventions
 *  
 * When another person is developing your code, the thing he/she needs
 * most is documentation. Working code comes only second. If the code
 * does not work, the user still has a document on what to except. If
 * the code works but has no documentation, it is not trivial to
 * verify that the code really works as intended. The value of
 * undocumented buggy code approaches zero.
 *
 * In Into, <a href="http://www.stack.nl/~dimitri/doxygen/">Doxygen</a>
 * is used to generate documentation. We won't go into syntactical
 * details here but define the general principles of documentation. 
 *
 * The most general principle is simple: everything that can be
 * considered as interface to the outer world must be formally
 * documented. This includes types (enum, class, typedef),
 * preprocessor macros, namespaces, and functions and variables in
 * namespaces or in public or protected sections of a class
 * declaration. For the private interface and for the code itself,
 * informal documentation is sufficient. But it must exist. It is a
 * good practice to comment first and then code.
 *
 * In %Qt, documentation is placed into .cc files. This has the
 * advantage that documentation has less effect on compilation time
 * compared to it being placed into the header. The disadvantage is
 * that inline functions, typedefs, class declarations, templates etc. 
 * are not in .cc. This either splits the documentation into many
 * locations or separates documentation from what is being documented.
 *
 * In Into, interface documentation is always in the header file. The
 * problem with compilation overhead can be tackled with <a
 * href="http://ccache.samba.org/">ccache</a>.
 *
 * The style of documentation tags is that of JavaDoc, not native
 * Doxygen. Each keyword is preceded by \@, not \\. Formal comment
 * blocks begin with a slash (/) plus a double asterisk (**). Each
 * line within the block begins with an asterisk. The end of a block
 * is marked with a single asterisk plus a slash. Here's how:
 *
 * @doccomment
 *  * Function description.
 *  *
 *  * @param value parameter description
 *  *
 *  * @return return value description
 *  *
 *  * @exception PiiException& on error
 * @endcomment
 *
 * Often, however, it is not necessary to formally document every
 * parameter and return value. If the purpose of a function can be
 * clearly told with a sentence or two, the full format just
 * complicates things. An example:
 *
 * @doccomment
 *  * This function returns the square root of its argument.
 * @endcomment
 *
 * In addition to standard Doxygen-style comments there are a few
 * specialities, which are summarized below.
 *
 * @par Inputs and Outputs
 *
 * In the documentation of a class derived from PiiOperation one
 * should use special documentation commands for inputs and outputs.
 * Here's how:
 *
 * @include InputOutputDoc.h
 * 
 * @par Sub-operations
 *
 * A compound operation that has children is supposed to list the
 * children by their object name. Like this:
 *
 * @include SubOperationDoc.h
 *
 * @par Plug-in dependencies
 *
 * A plug-in that depends on other plug-ins should explicitly list the
 * compile-time and run-time dependencies. An example:
 *
 * @include PluginDependenciesDoc.h
 *
 *
 * @par Informal Documentation
 *
 * Once again: comment your code. The format of comments within the
 * code is more or less free. The only requirement is that it is
 * written in English.
 *
 * If you leave a feature unimplemented or there is something that is
 * not completely clear to you, mark it like this:
 *
 * @comment PENDING
 *  * Explain what is missing or what needs to be checked.
 * @endcomment
 *
 * If the code works incorrectly, mark it like this:
 *
 * @comment FIXME
 *  * Bug description.
 * @endcomment
 *
 * In case a piece of code needs special care, works in an
 * unconventional way, or affects something else in a non-apparent
 * way, make others know:
 *
 * @comment NOTE
 *  * Changing this will break that.
 * @endcomment
 *
 * If a solution to a problem requires techniques that won't stand
 * critical judgement, save your face:
 *
 * @comment HACK
 *  * A really good excuse for using duct tape for repair.
 * @endcomment
 */
