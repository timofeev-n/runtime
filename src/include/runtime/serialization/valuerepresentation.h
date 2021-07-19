#pragma once

#include "valuerepresentation/primitiverepresentation.h"
#include "valuerepresentation/optionalrepresentation.h"
#include "valuerepresentation/arrayrepresentation.h"
#include "valuerepresentation/tuplerepresentation.h"
#include "valuerepresentation/dictionaryrepresentation.h"
#include "valuerepresentation/objectrepresentation.h"
#include "valuerepresentation/stdtypesrepresentation.h"


namespace Runtime {


template<typename T>
inline constexpr bool IsPrimitiveRepresentable = !(
	IsOptionalRepresentable<T> ||
	IsArrayRepresentable<T> ||
	IsTupleRepresentable<T> ||
	IsDictionaryRepresentable<T> ||
	IsObjectRepresentable<T> 
	);



}
