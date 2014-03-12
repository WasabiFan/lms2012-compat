/**
 
Copyright (c) 2014 National Instruments Corp.
 
This software is subject to the terms described in the LICENSE.TXT file
 
SDG
*/


#ifndef Instruction_h
#define Instruction_h

#include "BuildConfig.h"

namespace Vireo
{

//------------------------------------------------------------
// For small MCUs special annotation is needed to read data from flash.
#ifdef VIVM_HARVARD
    #ifdef PROGMEM
        #define _PROGMEM PROGMEM
        // AVR uses GCC which supports 'typeof' extension Cx standard would be decltype, tough arduinos
        // gcc does not yet have full support for this function
        #define _PROGMEM_PTR(__this_ptr, _field) ((typeof(__this_ptr->_field))pgm_read_word(&(__this_ptr->_field)))
    #else
        #error : Need defines to support harvard architechure
    #endif
#else
    #define _PROGMEM
    #define _PROGMEM_PTR(__this_ptr, _field)  (__this_ptr->_field)
#endif

//------------------------------------------------------------
// The minimal Instructions "object"
struct InstructionCore;

typedef InstructionCore* (VIVM_FASTCALL _PROGMEM *InstructionFunction) (InstructionCore*);

struct InstructionCore
{
	InstructionFunction  _function;
#ifndef VIREO_PACKED_INSTRUCTIONS
	InstructionCore* _next;
#endif
};
    
//------------------------------------------------------------
// A struct used for accessing any number or arguments in a non type strict way.
struct GenericInstruction : public InstructionCore
{
    void* _args[1];  // may be zero or more elements
};

//------------------------------------------------------------
// Struct for var arg functions
struct VarArgInstruction : public InstructionCore
{
    size_t _count;   // may be zero or more arguments in addition to the count
};
    
//------------------------------------------------------------
// Structs for various static argument count instructions
struct Instruction0 : public InstructionCore
{
};

//------------------------------------------------------------
// Each struct has a Next() method that can be used outside of the class to
// determine what the 'next' instruction is. If the instruction is derived
// from and extended it must be overridden, or else an incorrect result will
// be returned.
#ifdef VIREO_PACKED_INSTRUCTIONS
    #define NEXT_INSTRUCTION_METHOD()   inline InstructionCore* Next() { return this + 1; }
#else
    #define NEXT_INSTRUCTION_METHOD()   inline InstructionCore* Next() { return this->_next; }
#endif

template <class type0>
struct Instruction1 : public Instruction0
{
    type0* _p0;
};

template <class type0, class type1>
struct Instruction2 : public Instruction1<type0>
{
    type1* _p1;
    NEXT_INSTRUCTION_METHOD()
};

template <class type0, class type1, class type2>
struct Instruction3 : public Instruction2<type0, type1>
{
    type2* _p2;
    NEXT_INSTRUCTION_METHOD()
};

template <class type0, class type1, class type2, class type3>
struct Instruction4 : public Instruction3<type0, type1, type2>
{
    type3* _p3;
    NEXT_INSTRUCTION_METHOD()
};

template <class type0, class type1, class type2, class type3, class type4>
struct Instruction5 : public Instruction4<type0, type1, type2, type3>
{
    type4* _p4;
    NEXT_INSTRUCTION_METHOD()
};

template <class type0, class type1, class type2, class type3, class type4, class type5>
struct Instruction6 : public Instruction5<type0, type1, type2, type3, type4>
{
    type5* _p5;
    NEXT_INSTRUCTION_METHOD()
};

template <class type0, class type1, class type2, class type3, class type4, class type5, class type6>
struct Instruction7 : public Instruction6<type0, type1, type2, type3, type4, type5>
{
    type6* _p6;
    NEXT_INSTRUCTION_METHOD()
};

//------------------------------------------------------------
#ifdef VIREO_PACKED_INSTRUCTIONS
    // Standard VarArg functions have the param count in the beginning of the instruction.
    #define _NextInstructionV() ( (InstructionCore*) ((size_t*)((VarArgInstruction*)_this + 1) + (int)_this->_count) )
    #define _NextInstruction() (_this + 1)
#else
    #define _NextInstructionV() (_this->_next)
    #define _NextInstruction() (_this->_next)
#endif

//------------------------------------------------------------
// Macros to define/access arguments passed by pointers in the snippet
#define _ParamDef(t, n)         t* _p##n
#define _Param(n)               (*_PROGMEM_PTR(_this, _p##n))
#define _ParamPointer(n)        (_PROGMEM_PTR(_this, _p##n))

// Macros to define/access arguments stored directly in the snippet
#define _ParamImmediateDef(t, n) t _pi##n
#define _ParamImmediate(n)      (_PROGMEM_PTR(_this, _pi##n))

// Macro to access the arguments count variable in a variable argument functions.
// var-arg functions must derive from VarArgInstruction
// so the count will be the first argument in the snippet
#define _ParamVarArgCount()     ((int)_PROGMEM_PTR(_this, _count))
    
#ifdef _DEBUG
#define VIVM_TAIL_CALLS_USE_JMP 0
#else
#define VIVM_TAIL_CALLS_USE_JMP 1
#endif

#if  VIVM_TAIL_CALLS_USE_JMP
// Compiler supports direct jump to last function. Save trip back to exec.
#define VIVM_TAIL(__instruction)  ((__instruction)->_function(__instruction))
#else
// Arbitrarily deep recursion does not help, just return
#define VIVM_TAIL(__instruction)  (_this->_next)
#endif

//------------------------------------------------------------
// Macros to make type strict single parameter function declarations for snippets
// that take arguments passed in a param block
#define VIREO_FUNCTION_SIGNATURET(_name_, t)  \
    VIVM_INSTRUCTION_LINKAGE InstructionCore* VIVM_FASTCALL _name_ (t* _this _PROGMEM)

#define VIREO_FUNCTION_SIGNATUREV(_name_, _type_) \
    extern InstructionCore* VIVM_FASTCALL _name_ (_type_* _this _PROGMEM)

#define VIREO_FUNCTION_SIGNATURE0(_name_) \
    VIVM_INSTRUCTION_LINKAGE InstructionCore* VIVM_FASTCALL _name_ (InstructionCore* _this _PROGMEM)

#define VIREO_FUNCTION_SIGNATURE1(_name_, t0) \
    VIVM_INSTRUCTION_LINKAGE InstructionCore* VIVM_FASTCALL _name_ (Instruction1<t0>* _this _PROGMEM)

#define VIREO_FUNCTION_SIGNATURE2(_name_, t0, t1) \
    VIVM_INSTRUCTION_LINKAGE InstructionCore* VIVM_FASTCALL _name_ (Instruction2<t0, t1>* _this _PROGMEM)

#define VIREO_FUNCTION_SIGNATURE3(_name_, t0, t1, t2) \
    VIVM_INSTRUCTION_LINKAGE InstructionCore* VIVM_FASTCALL _name_ (Instruction3<t0, t1, t2>* _this _PROGMEM)

#define VIREO_FUNCTION_SIGNATURE4(_name_, t0, t1, t2, t3) \
    VIVM_INSTRUCTION_LINKAGE InstructionCore* VIVM_FASTCALL _name_ (Instruction4<t0, t1, t2, t3>* _this _PROGMEM)

#define VIREO_FUNCTION_SIGNATURE5(_name_, t0, t1, t2, t3, t4) \
    VIVM_INSTRUCTION_LINKAGE InstructionCore* VIVM_FASTCALL _name_ (Instruction5<t0, t1, t2, t3, t4>* _this _PROGMEM)

#define VIREO_FUNCTION_SIGNATURE6(_name_, t0, t1, t2, t3, t4, t5) \
    VIVM_INSTRUCTION_LINKAGE InstructionCore* VIVM_FASTCALL _name_ (Instruction6<t0, t1, t2, t3, t4, t5>* _this _PROGMEM)

#define VIREO_FUNCTION_SIGNATURE7(_name_, t0, t1, t2, t3, t4, t5, t6) \
    VIVM_INSTRUCTION_LINKAGE InstructionCore* VIVM_FASTCALL _name_ (Instruction7<t0, t1, t2, t3, t4, t5, t6>* _this _PROGMEM)

//------------------------------------------------------------
#define DECLARE_VIREO_PRIMITIVE0(_name_, _body_)	\
VIREO_FUNCTION_SIGNATURE0(_name_)				\
{										\
VIVM_TRACE_FUNCTION(#_name_)            \
(_body_);								\
return _NextInstruction();				\
}

#define DECLARE_VIREO_PRIMITIVE1(_name_, _t0_, _body_)	\
VIREO_FUNCTION_SIGNATURE1(_name_, _t0_)				\
{										\
VIVM_TRACE_FUNCTION(#_name_)            \
(_body_);								\
return _NextInstruction();				\
}

#define DECLARE_VIREO_PRIMITIVE2(_name_, _t0_, _t1_, _body_)	\
VIREO_FUNCTION_SIGNATURE2(_name_, _t0_, _t1_)				\
{										\
VIVM_TRACE_FUNCTION(#_name_)            \
(_body_);								\
return _NextInstruction();				\
}

#define DECLARE_VIREO_PRIMITIVE3(_name_, _t0_, _t1_, _t2_, _body_)	\
VIREO_FUNCTION_SIGNATURE3(_name_, _t0_, _t1_, _t2_)				\
{										\
VIVM_TRACE_FUNCTION(#_name_)            \
(_body_);								\
return _NextInstruction();              \
}

#define DECLARE_VIREO_PRIMITIVE4(_name_, _t0_, _t1_, _t2_, _t3_, _body_)	\
VIREO_FUNCTION_SIGNATURE4(_name_, _t0_, _t1_, _t2_, _t3_)				\
{										\
VIVM_TRACE_FUNCTION(#_name_)            \
(_body_);								\
return _NextInstruction();				\
}

// This relies on tail call optimization for both directions
// putting code in to check for back branches dynamically would be a waste.
// Yields should be added by the instruction thread builder for back branches.
// arg 0 is reserved for use as the "if true" target address
#define DECLARE_VIREO_CONDITIONAL_BRANCH(_name_, _t1_, _t2_, _test_) \
VIREO_FUNCTION_SIGNATURE3(_name_, InstructionCore, _t1_, _t2_) \
{									\
if (_test_)                         \
{									\
return _this->_p0;                  \
}									\
return VIVM_TAIL(_NextInstruction()); \
}

// This relies on tail call optimization for both directions
// putting code in to check for back branches dynamically would be a waste.
// Yields should be added by the instruction thread builder for back branches.
// arg 0 is reserved for use as the "if true" target address
#define DECLARE_VIREO_CONDITIONAL_BRANCH1(_name_, _t1_,  _test_) \
VIREO_FUNCTION_SIGNATURE2(_name_, InstructionCore, _t1_) \
{									\
if (_test_)                         \
{									\
return _this->_p0;                  \
}									\
return VIVM_TAIL(_NextInstruction()); \
}

} // namespace Vireo
#endif //Instruction_h
