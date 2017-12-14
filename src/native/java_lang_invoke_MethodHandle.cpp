/*
 * java_lang_invoke_MethodHandle.cpp
 *
 *  Created on: 2017年12月13日
 *      Author: zhengxiaolin
 */

#include "native/java_lang_invoke_MethodHandle.hpp"
#include "native/java_lang_invoke_MethodHandleNatives.hpp"
#include <vector>
#include <algorithm>
#include <cassert>
#include "native/native.hpp"
#include "native/java_lang_String.hpp"
#include "utils/os.hpp"
#include "classloader.hpp"
#include "wind_jvm.hpp"

static unordered_map<wstring, void*> methods = {
    {L"invoke:([" OBJ ")" OBJ,								(void *)&JVM_Invoke},
};

void argument_unboxing(list<Oop *> & args)		// Unboxing args for Integer, Double ... to int, double, etc.
{
	list<Oop *> temp;
	for (Oop *oop : args) {
		if (oop == nullptr) {
			temp.push_back(nullptr);
			continue;
		} else {
			InstanceOop *real_oop = (InstanceOop *)oop;
			wstring klass_name = oop->get_klass()->get_name();
			Oop *ret;
			if (klass_name == BYTE0) {
				real_oop->get_field_value(BYTE0 L":value:B", &ret);
				temp.push_back(new IntOop(((IntOop *)ret)->value));
			} else if (klass_name == BOOLEAN0) {
				real_oop->get_field_value(BOOLEAN0 L":value:Z", &ret);
				temp.push_back(new IntOop(((IntOop *)ret)->value));
			} else if (klass_name == SHORT0) {
				real_oop->get_field_value(SHORT0 L":value:S", &ret);
				temp.push_back(new IntOop(((IntOop *)ret)->value));
			} else if (klass_name == CHARACTER0) {
				real_oop->get_field_value(CHARACTER0 L":value:C", &ret);
				temp.push_back(new IntOop(((IntOop *)ret)->value));
			} else if (klass_name == INTEGER0) {
				real_oop->get_field_value(INTEGER0 L":value:I", &ret);
				temp.push_back(new IntOop(((IntOop *)ret)->value));
			} else if (klass_name == FLOAT0) {
				real_oop->get_field_value(FLOAT0 L":value:F", &ret);
				temp.push_back(new FloatOop(((FloatOop *)ret)->value));
			} else if (klass_name == DOUBLE0) {
				real_oop->get_field_value(DOUBLE0 L":value:D", &ret);
				temp.push_back(new DoubleOop(((DoubleOop *)ret)->value));
			} else if (klass_name == LONG0) {
				real_oop->get_field_value(LONG0 L":value:J", &ret);
				temp.push_back(new LongOop(((LongOop *)ret)->value));
			} else {
				temp.push_back(real_oop);
			}
		}
	}
	temp.swap(args);
}

InstanceOop *return_val_boxing(Oop *basic_type_oop, vm_thread *thread)
{
	if (basic_type_oop->get_ooptype() != OopType::_BasicTypeOop)	return nullptr;

	shared_ptr<Method> target_method;
	switch (((BasicTypeOop *)basic_type_oop)->get_type()) {
		case Type::BOOLEAN: {
			auto box_klass = std::static_pointer_cast<InstanceKlass>(BootStrapClassLoader::get_bootstrap().loadClass(BOOLEAN0));
			target_method = box_klass->get_this_class_method(L"valueOf:(Z)Ljava/lang/Boolean;");		// static
			assert(target_method != nullptr);
			return (InstanceOop *)thread->add_frame_and_execute(target_method, {new IntOop(((IntOop *)basic_type_oop)->value)});
		}
		case Type::BYTE: {
			auto box_klass = std::static_pointer_cast<InstanceKlass>(BootStrapClassLoader::get_bootstrap().loadClass(BYTE0));
			target_method = box_klass->get_this_class_method(L"valueOf:(B)Ljava/lang/Byte;");		// static
			assert(target_method != nullptr);
			return (InstanceOop *)thread->add_frame_and_execute(target_method, {new IntOop(((IntOop *)basic_type_oop)->value)});
		}
		case Type::SHORT: {
			auto box_klass = std::static_pointer_cast<InstanceKlass>(BootStrapClassLoader::get_bootstrap().loadClass(SHORT0));
			target_method = box_klass->get_this_class_method(L"valueOf:(S)Ljava/lang/Short;");		// static
			assert(target_method != nullptr);
			return (InstanceOop *)thread->add_frame_and_execute(target_method, {new IntOop(((IntOop *)basic_type_oop)->value)});
		}
		case Type::CHAR: {
			auto box_klass = std::static_pointer_cast<InstanceKlass>(BootStrapClassLoader::get_bootstrap().loadClass(CHARACTER0));
			target_method = box_klass->get_this_class_method(L"valueOf:(C)Ljava/lang/Character;");		// static
			assert(target_method != nullptr);
			return (InstanceOop *)thread->add_frame_and_execute(target_method, {new IntOop(((IntOop *)basic_type_oop)->value)});
		}
		case Type::INT: {
			auto box_klass = std::static_pointer_cast<InstanceKlass>(BootStrapClassLoader::get_bootstrap().loadClass(INTEGER0));
			target_method = box_klass->get_this_class_method(L"valueOf:(I)Ljava/lang/Integer;");		// static
			assert(target_method != nullptr);
			return (InstanceOop *)thread->add_frame_and_execute(target_method, {new IntOop(((IntOop *)basic_type_oop)->value)});
		}
		case Type::FLOAT: {
			auto box_klass = std::static_pointer_cast<InstanceKlass>(BootStrapClassLoader::get_bootstrap().loadClass(FLOAT0));
			target_method = box_klass->get_this_class_method(L"valueOf:(F)Ljava/lang/Float;");		// static
			assert(target_method != nullptr);
			return (InstanceOop *)thread->add_frame_and_execute(target_method, {new FloatOop(((FloatOop *)basic_type_oop)->value)});
		}
		case Type::LONG: {
			auto box_klass = std::static_pointer_cast<InstanceKlass>(BootStrapClassLoader::get_bootstrap().loadClass(LONG0));
			target_method = box_klass->get_this_class_method(L"valueOf:(J)Ljava/lang/Long;");		// static
			assert(target_method != nullptr);
			return (InstanceOop *)thread->add_frame_and_execute(target_method, {new LongOop(((LongOop *)basic_type_oop)->value)});
		}
		case Type::DOUBLE: {
			auto box_klass = std::static_pointer_cast<InstanceKlass>(BootStrapClassLoader::get_bootstrap().loadClass(DOUBLE0));
			target_method = box_klass->get_this_class_method(L"valueOf:(D)Ljava/lang/Double;");		// static
			assert(target_method != nullptr);
			return (InstanceOop *)thread->add_frame_and_execute(target_method, {new DoubleOop(((DoubleOop *)basic_type_oop)->value)});
		}
		default:
			assert(false);
	};
}

void JVM_Invoke(list<Oop *> & _stack){
	// 注意！！接下来是硬编码！因为 _stack 最后两个，为了防止 native 函数内部还要调用 java 方法，以及在 static native 方法得到调用者的 klass，我当时在 _stack 的后边默认加了两个参数：
	// 即，_stack 的内存模型应该是这样：(假设长度为 length，即下标范围: [0~length-1])
	// [0]. _this(如果此方法为 native non-static 的话，有此 _this 参数)
	// [1]. Arg0
	// [2]. Arg1
	//	...
	// [length-2]: CallerKlassMirror *
	// [length-1]: vm_thread * (此 native method 所在的线程栈)
	// 之所以会有后两个，就是因为以上原因。
	// 而这里，由于参数不定长度，因此必须计算出来 _stack.size()，然后 -2，再 -1 (减去 _this)，即是所有参数的长度。
	// 而日后不知道会不会需要改进，而在 [length-3] 处放置 Caller 的其他信息。
	// 所以特此留下说明。届时需要把 _stack.size() - 2 - 1 改成 _stack.size() - 3 - 1, etc.

	InstanceOop *_this = (InstanceOop *)_stack.front();	_stack.pop_front();		// pop 出 [0] 的 _this
	vm_thread *thread = (vm_thread *)_stack.back();	_stack.pop_back();			// pop 出 [length-1] 的 vm_thread
	_stack.pop_back();															// pop 出 [length-2] 的 CallerKlassMirror *.
	// 现在 _stack 剩下的全是参数～。

	Oop *oop;
	_this->get_field_value(METHODHANDLE L":type:Ljava/lang/invoke/MethodType;", &oop);
	InstanceOop *methodType = (InstanceOop *)oop;

	// get the **REAL MemberName** from Table through methodType...
	InstanceOop *member_name_obj = find_table_if_match_methodType(methodType);

	// 非常悲伤。因为研究了好长时间也没有研究出来那个 vmindex 和 vmtarget 到底放在哪。感觉应该是 jvm 对 MemberName 这个类钦定的吧......
	// 所以这里只能重新查找了......QAQ
	// 所以下边的代码调用了 JVM_Resolve 的大部分函数......
	// 在每次解析出来 MemberName 的时候，都要把它放到一个 Table 中！！这个 MethodName 中自带一个 MethodType，这个 MethodType 的对象或许能用......
	// 应该可以和 MethodHandle 中的 MethodType 查地址然后配对......(已采用)

	member_name_obj->get_field_value(MEMBERNAME L":clazz:" CLS, &oop);
	MirrorOop *clazz = (MirrorOop *)oop;		// e.g.: Test8
	assert(clazz != nullptr);
	member_name_obj->get_field_value(MEMBERNAME L":name:" STR, &oop);
	InstanceOop *name = (InstanceOop *)oop;	// e.g.: doubleVal
	assert(name != nullptr);
	member_name_obj->get_field_value(MEMBERNAME L":type:" OBJ, &oop);
	InstanceOop *type = (InstanceOop *)oop;	// maybe a String, or maybe an Object[]...
	assert(type != nullptr);
	member_name_obj->get_field_value(MEMBERNAME L":flags:I", &oop);
	int flags = ((IntOop *)oop)->value;

	auto klass = clazz->get_mirrored_who();

	// decode is from openjdk:

	int ref_kind = ((flags & 0xF000000) >> 24);
	/**
	 * from 1 ~ 9:
	 * 1: getField
	 * 2: getStatic
	 * 3: putField
	 * 4: putStatic
	 * 5: invokeVirtual
	 * 6: invokeStatic
	 * 7: invokeSpecial
	 * 8: newInvokeSpecial
	 * 9: invokeInterface
	 */
	assert(ref_kind >= 5 && ref_kind <= 9);		// must be method call...

	auto real_klass = std::static_pointer_cast<InstanceKlass>(klass);
	wstring real_name = java_lang_string::stringOop_to_wstring(name);
	if (real_name == L"<clinit>" || real_name == L"<init>") {
		assert(false);		// can't be the two names.
	}

	// 0. create a empty wstring: descriptor
	wstring descriptor = get_member_name_descriptor(real_klass, real_name, type);
	// 1. get the signature
	wstring signature = real_name + L":" + descriptor;
	// 2. get the target method
	shared_ptr<Method> target_method = get_member_name_target_method(real_klass, signature, ref_kind);

	//	if (ref_kind == 6)	{			// invokeStatic
//
//	} else if (ref_kind == 5) { 		// invokeVirtual
//
//	} else if (ref_kind == 7) {		// invokeSpecial
//		assert(false);		// not support yet...
//	} else if (ref_kind == 9) {		// invokeInterface
//		assert(false);		// not support yet...
//	} else {
//		assert(false);
//	}

	// simple check
	int size = BytecodeEngine::parse_arg_list(target_method->get_descriptor()).size();
	assert(size == _stack.size());		// 参数一定要相等......

	// 把参数所有的自动装箱类解除装箱...... 因为 invoke 的参数全是 Object，而真实的参数可能是 int。
	argument_unboxing(_stack);

	// 3. call it!		// return maybe: BasicTypeOop, ArrayOop, InstanceOop... all.
	Oop *result = thread->add_frame_and_execute(target_method, _stack);		// TODO: 如果抛了异常。

	// 把返回值所有能自动装箱的自动装箱......因为要返回一个 Object。
	Oop *real_result = return_val_boxing(result, thread);

	_stack.push_back(real_result);
}


// 返回 fnPtr.
void *java_lang_invoke_methodHandle_search_method(const wstring & signature)
{
	auto iter = methods.find(signature);
	if (iter != methods.end()) {
		return (*iter).second;
	}
	return nullptr;
}