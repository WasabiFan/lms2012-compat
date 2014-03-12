/**
 
Copyright (c) 2014 National Instruments Corp.
 
This software is subject to the terms described in the LICENSE.TXT file
 
SDG
*/


#ifndef VirtualInstrument_h
#define VirtualInstrument_h

#include "DataTypes.h"
#include "EventLog.h"

namespace Vireo
{

class VIClump;

#define VI_TypeName             "VirtualInstrument"
#define ReentrantVI_TypeName    "ReentrantVirtualInstrument"

//------------------------------------------------------------
#define VI_TypeString               \
"a(c(                               \
    e(.ExecutionContext Context)    \
    e(a(.*) ParamBlock)             \
    e(a(.*) DataSpace)              \
    e(a(.VIClump *) Clumps)         \
    e(.Int32 lineNumberBase)        \
    e(.DataPointer ClumpSourceBegin)\
    e(.DataPointer ClumpSourceEnd)  \
))"

class VIDataProcsClass;
    
class VirtualInstrument
{
    friend class VIDataProcsClass;
private:
    ExecutionContext*       _executionContext;
    TypedBlock*             _paramBlock;          // All clumps in subVI share the same param block
    TypedBlock*             _dataSpace;           // All clumps in subVI share the same data
    TypedArray1D<VIClump>*  _clumps;
public:
    Int32                   _lineNumberBase;
    SubString               _clumpSource;         // For now, this is tied to the VIA codec. It has a Begin and End pointer
public :
    NIError Init(ExecutionContext* context, Int32 clumpCount, TypeRef paramBlockType, TypeRef dataSpaceType, Int32 lineNumberBase, SubString* source);
    void PressGo();
public:
    VirtualInstrument(ExecutionContext *context, int clumps, TypeRef paramBlockType, TypeRef dataSpaceType);
    ExecutionContext* OwningContext()   {return _executionContext;}
    TypedArrayCore* ParamBlock()        {return _paramBlock;}
    TypedArrayCore* DataSpace()         {return _dataSpace;}
    TypedArray1D<VIClump>* Clumps()     {return _clumps;}
};

typedef TypedArray1D<VirtualInstrument> VirtualInstrumentObject;

class FunctionClump
{
public:
	InstructionCore* _codeStart;        // first instruction object in clump. may be shared  between multiple QEs
};

//------------------------------------------------------------
#define VIClump_TypeString              \
"c(                                     \
    e(.InstructionList CodeStart)       \
    e(.DataPointer Next)                \
    e(.DataPointer Owner)               \
    e(.DataPointer NextWaitingCaller)   \
    e(.DataPointer Caller)              \
    e(.Instruction SavePC)              \
    e(.Int64 WakeUpInfo)                \
    e(.Int32 FireCount)                 \
    e(.Int32 ShortCount)                \
)"

// A QueueElt (QE) will be allocated for every VI and every sub clump for VIs that have parallelism.
// Reentrant callers need to maintain separate state so a copy is needed for each call site.
class VIClump : public FunctionClump
{
public:
    VIClump*            _next;              // Next element in Q this clump is part of.
    VirtualInstrument*  _owningVI;
	VIClump*            _waitingClumps;     // If this clump is busy when called then callers are linked here.
	VIClump*            _caller; 			// Used for sub vi calls, clump to restart once done.
	InstructionCore*    _savePc;            // Save when paused either due to sub vi call, or time slicing
	uTicTimeType        _wakeUpInfo;		// If clump is suspended, used to determine if wake up condition exists (e.g. time)
	CounterType         _fireCount;         // What to reset shortCount to.
	CounterType         _shortCount;		// Greater than 0 is not in run queue, when it goes to zero it gets enqueued
    
public:
    void Trigger();
    CounterType         FireCount() { return _fireCount;}
    CounterType         ShortCount() { return _shortCount;}
    
    void InsertIntoWaitList(VIClump* elt);
    VirtualInstrument*  OwningVI() {return _owningVI;};
    TypeManager*        TheTypeManager();    
};

//------------------------------------------------------------
// The SubVI Call instruction contains a pointer tot he root clump
// and sub snippets for parameters in and out.
struct CallVIInstruction : public InstructionCore
{
    _ParamImmediateDef(VIClump*, viRootClump);

#ifdef VIREO_PACKED_INSTRUCTIONS
    // In packed mode, the adjacent instruction is the first copy-in
    // instruction. The instruction following the subVI call has its own
    // explicit field.
    _ParamImmediateDef(InstructionCore*, next);
    inline InstructionCore* CopyInSnippet()    { return this + 1; }
    inline InstructionCore* Next()             { return this->_pinext; }
#else
    // In unpacked mode all instructions have their own pointer
    _ParamImmediateDef(InstructionCore*, copyInSnippet);
    inline InstructionCore* CopyInSnippet()    { return this->_picopyInSnippet; }
    NEXT_INSTRUCTION_METHOD()
#endif

    _ParamImmediateDef(InstructionCore*, copyOutSnippet);

};
//------------------------------------------------------------
class InstructionAllocator {
public:
    size_t      _size;
    AQBlock1*   _next;
    
    InstructionAllocator() { _size = 0; _next = null; }
    void AddRequest(size_t count);
    void Allocate (TypeManager * tm);
    void* AllocateSlice(size_t count);
};
//------------------------------------------------------------
struct PatchInfo
{
    enum PatchType {
        Perch = 0,
        NamedTypes = 1,
    };
    
    PatchType   _patchType;
    void**      _whereToPeek;
    void**      _whereToPatch;
};
//------------------------------------------------------------
class ClumpParseState
{
    // The compiler (clang) really did not want to allow static const pointers so they are #defines
#define kPerchUndefined     ((InstructionCore*)0)    // What a branch sees when it is used before a perch
#define kPerchBeingAlocated ((InstructionCore*)1)    // Perches awaiting the next instruction address see this
    static const Int32 kMaxPerches = 200;   // TODO dynamic
    static const Int32 kMaxArguments = 100; // TODO dynamic
    static const Int32 kMaxPatchInfos = 100; // TODO allow more
public:
    enum ArgumentState {
        // Initial state, not where it should end in either.
        kArgumentNotResolved,
        // Bad states to end in
        kArgumentTooMany,
        kArgumentTooFew,
        kArgumentTypeMismatch,
        kArgumentNotOptional,
        kArgumentNotMutable,
        // Good states to end in
        kArgumentResolved_FirstGood,
        kArgumentResolvedToLocal = kArgumentResolved_FirstGood,
        kArgumentResolvedToGlobal,
        kArgumentResolvedToParameter,
        kArgumentResolvedToClump,
        kArgumentResolvedToPerch,
        kArgumentResolvedToStaticString,
        kArgumentResolvedToInstructionFunction,
        kArgumentResolved_LastGood = kArgumentResolvedToStaticString,
    };
    ArgumentState _argumentState;
    EventLog*       _pLog;
    InstructionAllocator* _cia;
    
    Int32           _argCount;
    void*           _argPointers[kMaxArguments];
    TypeRef         _argTypes[kMaxArguments];
    
    Int32           _argPatches[kMaxArguments];     // Arguments that need patching
    Int32           _argPatchCount;
    
    PatchInfo       _patchInfos[kMaxPatchInfos];
    Int32           _patchInfoCount;
    
    Int32           _perchCount;
    InstructionCore* _perches[kMaxPerches];
    
    VirtualInstrument *_vi;
    VIClump*        _clump;

private:    // State to pathcing owner/next field once next instruction created
    // When an instruction is made remember where its 'next' field is so that it can be
    // when the next instruction is generated. When packed instructions are used
    // this is most often null since there are no next pointers. However roots
    // still exist like the pointer in the clump that points to the first instruction,
    // or a sub snippet pointer in the CallVI instruction.
    InstructionCore**   _pWhereToPatch;
    void                StartBuilding(VIClump* clump, InstructionCore** startLocation, InstructionAllocator *);
    void                RecordNextHere(InstructionCore** startLocation);
    InstructionCore*    AllocInstructionCore(Int32 argumentCount);
    InstructionCore*    CreateInstruction(TypeRef instructionType, Int32 argCount, void* args[]);

private:    // State related to two pass parseing
    Int32           _totalInstructionCount;
    Int32           _totalInstructionPointerCount;

private:    // state related to the the current argument
    Int32           _formalParameterIndex;
    TypeRef         _formalParameterType;
    TypeRef         _actualArgumentType;
    
public:
    SubString       _actualArgumentName;

    // ----
    // State related to the clumps owning VI
public:
    TypeRef         _paramBlockType;    // TODO get from _paramBlock ???
    AQBlock1*       _paramBlockBase;
    TypedBlock*     _paramBlock;
    // ----
    TypeRef         _dataSpaceType;     // TODO get from _paramBlock ???
    AQBlock1*       _dataSpaceBase;
    TypedBlock*     _dataSpace;
    
    // ---
    // The type that has the pointer to the specific target of the function.
    TypeRef         _instructionPointerType;
    
    // The calling signature descriptor for the instruction's function. Its the BaseType Of the PointerType.
    TypeRef         _instructionType;
    
    // When a Perch instruction is found the target address will be the next instruction
    // _recordNextInstructionAddress  lets the state know when that patch-up needed.
    Int32           _recordNextInstructionAddress;
    
    size_t*          _pVarArgCount;
    Boolean         _bIsVI;
    
    //------------------------------------------------------------
    ClumpParseState(ClumpParseState* cps);
    ClumpParseState(VIClump* clump, InstructionAllocator* cia, EventLog* pLog);
    void            Construct(VIClump* clump, InstructionAllocator* cia, EventLog* pLog);
    void            StartSnippet(InstructionCore** startLocation);
    TypeRef         FormalParameterType()       { return _formalParameterType; }
    TypeRef         ActualArgumentType()        { return _actualArgumentType; }
    Boolean         LastArgumentError()         { return _argumentState < kArgumentResolved_FirstGood; }
    TypeRef         ReadFormalParameterType();
    void            SetClumpFireCount(Int32 fireCount);
    TypeRef         StartInstruction(SubString* opName);
    TypeRef         ReresolveInstruction(SubString* opName, bool allowErrors);
    void            ResolveActualArgumentAddress(SubString* argument, AQBlock1** ppData);
    void            AddDataTargetArgument(SubString* argument, Boolean prependType);
    void            AddStaticString(SubString* argument);
    void            InternalAddArg(TypeRef actualType, void* arg);
    void            InternalAddArgNeedingPatch(PatchInfo::PatchType patchType, void** whereToPeek);
    Boolean         VarArgParameterDetected()   { return _pVarArgCount != null; }
    Boolean         GenericFunction()           { return _instructionType->HasGenericType(); }
    void            AddVarArgCount();
    void            MarkPerch(SubString* perchToken);
    void            AddBranchTargetArgument(SubString* branchTargetToken);
    void            AddClumpTargetArgument(SubString* clumpIndexToken);
    void            AddInstructionFunctionArgument(SubString* instructionNameToken);
    VirtualInstrument*  AddSubVITargetArgument(SubString* subVIName);
    Int32           AddSubSnippet();
    void            LogArgumentProcessing(Int32 lineNumber);
    InstructionCore*    EmitCallVIInstruction();
    InstructionCore*    EmitInstruction();
    void            EmitSimpleInstruction(const char* opName);
    void            CommitSubSnippet();
    void            CommitClump();
    void            BeginEmitSubSnippet(ClumpParseState* subSnippet, InstructionCore* owningInstruction, Int32 argIndex);
    void            EndEmitSubSnippet(ClumpParseState* subSnippet);
};

typedef InstructionCore* (VIVM_FASTCALL _PROGMEM *GenericEmitFunction) (ClumpParseState*);

}
#endif //VirtualInstrument_h

