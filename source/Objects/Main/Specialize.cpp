#include "alif.h"
#include "OpCode.h"
#include "AlifCore_StackRef.h"
#include "AlifCore_Code.h"









void alifCode_quicken(AlifCodeObject* _code) { 
#if ENABLE_SPECIALIZATION
    int opcode = 0;
    AlifCodeUnit* instructions = ALIFCODE_CODE(_code);
    for (int i = 0; i < ALIF_SIZE(_code); i++) {
        opcode = alif_getBaseOpCode(_code, i);
        //int caches = alifOpCodeCaches[opcode];
        //if (caches) {
        //    switch (opcode) {
        //    case JUMP_BACKWARD:
        //        instructions[i + 1].counter = initial_jumpBackoffCounter();
        //        break;
        //    case POP_JUMPIF_FALSE:
        //    case POP_JUMPIF_TRUE:
        //    case POP_JUMPIF_NONE:
        //    case POP_JUMPIF_NOTNONE:
        //        instructions[i + 1].cache = 0x5555;  // Alternating 0, 1 bits
        //        break;
        //    default:
        //        instructions[i + 1].counter = adaptive_counterWarmup();
        //        break;
        //    }
        //    i += caches;
        //}
    }
#endif /* ENABLE_SPECIALIZATION */
}

#define SPEC_FAIL_SUPER_BAD_CLASS 9
#define SPEC_FAIL_SUPER_SHADOWED 10

void alifSpecialize_loadSuperAttr(AlifStackRef global_super_st, AlifStackRef cls_st, AlifCodeUnit* instr, int load_method) {
	AlifObject* global_super = ALIFSTACKREF_ASALIFOBJECTBORROW(global_super_st);
	AlifObject* cls = ALIFSTACKREF_ASALIFOBJECTBORROW(cls_st);

	AlifSuperAttrCache* cache = (AlifSuperAttrCache*)(instr + 1);
	if (global_super != (AlifObject*)&_alifSuperType_) {
		//SPECIALIZATION_FAIL(LOAD_SUPER_ATTR, SPEC_FAIL_SUPER_SHADOWED);
		goto fail;
	}
	if (!ALIFTYPE_CHECK(cls)) {
		//SPECIALIZATION_FAIL(LOAD_SUPER_ATTR, SPEC_FAIL_SUPER_BAD_CLASS);
		goto fail;
	}
	instr->op.code = load_method ? LOAD_SUPER_ATTR_METHOD : LOAD_SUPER_ATTR_ATTR;
	goto success;

fail:
	//STAT_INC(LOAD_SUPER_ATTR, failure);
	instr->op.code = LOAD_SUPER_ATTR;
	cache->counter = adaptive_counter_backoff(cache->counter);
	return;
success:
	//STAT_INC(LOAD_SUPER_ATTR, success);
	cache->counter = adaptive_counter_cooldown();
}
