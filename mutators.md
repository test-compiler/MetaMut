# Unsupervised Mutators

| name | description |
|------|-------------|
|ConvertGlobalToLocal|Converts a global variable to local variables in all functions where it is used|
|ConvertIfToSwitch|Converts simple if-else-if ladders into a switch-case construct|
|DecayRandomArrayToPointer|Decay a randomly selected array variable to a pointer variable.|
|DecayRandomFunctionToPointer|Decays random functions to pointers and adjusts call sites accordingly.|
|DecayRandomPointerToArray|Change a pointer declaration to an array declaration.|
|DuplicateBranch|Duplicates one branch of an if statement|
|DuplicateRandomUnaryOperator|Duplicate randomly selected UnaryOperator.|
|EmbedCompoundStmt|Embed a CompoundStmt inside another CompoundStmt|
|FlipLogicalOperator|Flip the logical operator in a LogicalExpr.|
|InlineFunctionCall|Inline a function call with its body.|
|InlineRandomVarDecl|Inline a random VarDecl's initializer and remove the declaration.|
|InsertRandomBreakStmt|Randomly inserts a break statement in a loop body.|
|InsertRandomReturnStmt|Insert a return statement at a random position in a function|
|InsertRandomSwitchStmt|Replace if-else-if ladder with a switch-case construct.|
|InterchangeFuncCalls|Interchange positions of two function calls within a CompoundStmt|
|IntroduceRandomTernaryOperator|Randomly selects a BinaryOperator in a Boolean condition and converts it into a ternary operator using an equivalent logical expression.|
|InverseLogicalExpr|Inverse logical expressions.|
|InvertFunctionCondition|Inverts the condition of a boolean function.|
|InvertRelationalOperator|Invert the relational operator in a BinaryOperator|
|LiftNestedFunction|Lifts a function call to the outer level and replaces the original location with a new variable.|
|ModifyFunctionReturnTypeToVoid|Change a function's return type to void, remove all return statements, and replace all uses of the function's result with a default value.|
|ModifyPointerTarget|Modifies the target of a random PointerType variable to another integer variable available in the current scope.|
|ModifyRandomEnum|Randomly select an EnumDecl and change the value of one of its EnumConstantDecls to a different integer value.|
|ModifyRandomLiteralInExpr|Modify a random literal in an expression statement to another random value of the same type.|
|MutateVariableInitializationToRandomFunctionCall|Replace variable initialization with a random function call.|
|NegateLiteral|Negate a selected Integer or Floating point Literal.|
|RandomArrayElementAssign|Randomly selects an ArrayType variable and assigns a random element to a new value of the same type.|
|RandomizeFunctionOrder|Randomly swap two function definitions in a program.|
|RandomizeIfElseBranches|Randomly selects an IfStmt, inverts the condition, and swaps the then and else branches.|
|RemoveRandomParameter|Randomly remove a non-essential parameter from a function declaration and adjust all call sites accordingly.|
|ReplaceArrayWithPointer|Change an ArrayDecl to PointerDecl and modify all relevant array access expressions to pointer dereference.|
|ReplaceIntWithEquivalentBitwiseExpr|Replace integer with equivalent bitwise expression, excluding return statements.|
|ReplaceLiteralWithConstant|Replace a literal with a global constant.|
|ReplaceLiteralWithRandomValue|Replace a literal with a random value of the same type.|
|ReplaceRandomCompoundAssignWithSeparate|Randomly selects a CompoundAssignOperator (like +=, -=, \*=, /=), breaks it down into a separate binary operation and an assignment, and replaces the original CompoundAssignOperator with the two new separate statements.|
|ReplaceRandomExprWithNullptr|Replace a random pointer expression with NULL.|
|ReplaceRandomOperatorWithInverse|Replace a binary operator with its inverse and adjust the right hand side operand to maintain the original result.|
|ReplaceRandomTypeWithTypedef|Randomly selects a Type and replaces it with a typedef of the same underlying type.|
|ReplaceSqrtWithFabs|Replace calls to the sqrt function with calls to the fabs function.|
|ReplaceWhileWithDoWhile|Replace a WhileStmt with a DoWhileStmt, retaining the condition and body.|
|ReplaceWithEquivalentUnaryExpr|Replace UnaryExpr with an equivalent UnaryExpr that uses a different operator.|
|SplitCompoundStmt|Randomly split a CompoundStmt into two nested CompoundStmts.|
|SwapFuncDeclParams|Swap two parameters in a function declaration and its call sites|
|SwapNestedFunctionCalls|Swap the positions of two nested function calls.|
|SwapNestedIfElse|Swap a nested if-else statement and inverse the condition.|
|SwapRandomBinaryOperands|Swap operands of a random BinaryOperator's expression.|
|SwitchInitExpr|Switch initialization expressions of two VarDecls.|
|ToggleLoopType|Toggle between `for` and `while` loop types.|
|ToggleReturnExpr|Toggle return expression to default value.|
|ToggleStaticVarDecl|Toggle static and non-static for VarDecl|
|TransformIfToGuardClause|Transform IfStmt into a guard clause by inverting the condition and making the 'then' branch an early return.|

# Supervised Mutators

| name | description |
|------|-------------|
|add-initialization|Randomly add initialization to an uninitialized VarDecl.|
|add-suffix-unaryop|Randomly add a suffix unary operator.|
|aggregate-member-to-scalar-variable|Replace accesses to an aggregate member with a corresponding scalar variable.|
|change-charliteral|Change a CharacterLiteral's value.|
|change-declref|Change a DeclRefExpr's referenced variable.|
|change-decltype|Change a VarDecl's type to a compliant one.|
|change-fielddecltype|Change a FieldDecl's type to a compliant one.|
|change-floatliteral|Change a FloatingLiteral's value.|
|change-integer-literal|Change an integer literal.|
|change-looptype|Change a loop type ('for' to 'while' and vice versa).|
|change-param-scope|Change a parameter's scope.|
|change-parmtype|Change a ParmVarDecl's type to a compliant one.|
|change-return-value|Change a function's return value.|
|change-returntype|Change a FunctionDecl's return type to a compliant one.|
|change-vardecl-qualifier|Change a variable's type qualifier.|
|change-vardecl-scope|Change a variable's scope.|
|combine-variable|Combine variables into a struct.|
|copy-expr|Copy expression to a variable or constant literal.|
|copy-propagation|Propagate the RHS expression of assignment|
|decay-body-to-return|Remove all stmts inside a function's body except for DeclStmt and ReturnStmt.|
|decay-small-struct|Decay small struct to long long.|
|destruct-record|Break struct/union variables into smaller variables.|
|duplicate-fields|Duplicate a field of a struct or union type.|
|duplicate-parameter|Duplicate a function's parameter.|
|duplicate-stmt|Duplicate a random non-declaration statement.|
|duplicate-with-binop|Duplicate an expression with a binary operator.|
|duplicate-with-unop|Duplicate an expression with a unary operator.|
|flatten-array|Flatten multi-dimensional arrays to one dimension.|
|group-stmts-into-compound|Group a list of continuous stmt into compound stmt.|
|if-to-goto|Convert an if-else statement to goto statements.|
|if-to-switch|Convert a series of if-else statements to a switch statement.|
|increase-array-dimension|Add a dimension to an array variable.|
|increase-pointer-level|Increase a variable's pointer level.|
|inverse-inline|Toggle inline attributes of function declarations.|
|lift-logical-subexpr|Lift a subexpression from a logical expression.|
|modify-forloopincrement|Modify the increment step in a For loop.|
|mutate-binop|Change binary expression's operator.|
|mutate-suffix-unaryop|Change a UnaryOperator's suffix operator.|
|mutate-unop|Change unary expression's operator.|
|negate-branch-cond|Negate the condition of a branch statement.|
|random-copy-propagation|Propagate the RHS expression of assignment|
|randomize-string-literal|Randomly add/delete/duplicate characters in a string literal.|
|reduce-array-dimension|Change a dimension n array variable to dimension n-1 array.|
|reduce-pointer-level|Reduce a variable's pointer level.|
|remove-initialization|Randomly remove the initialization of a VarDecl.|
|remove-parameter|Remove a function's parameter.|
|remove-stmt|Remove a statement.|
|remove-suffix-unaryop|Remove a UnaryOperator's suffix operator.|
|remove-unop|Remove unary expression's operator.|
|remove-unused-fields|Remove unused fields of a struct or union type.|
|remove-unused-function|Remove an unused function.|
|remove-unused-parameter|Remove a function's unused parameter.|
|remove-unused-variable|Remove an unused variable.|
|shuffle-exprstmts|Shuffle continuous non-declaration statements.|
|shuffle-fields|Shuffle fields of a record.|
|shuffle-parameters|Shuffle a function's parameters.|
|shuffle-stmts|Shuffle continuous non-declaration statements.|
|simple-inliner|Inline function calls.|
|simple-uninliner|Turn a block of code into a function call.|
|strip-return|Change a function's return type to void and strip return keyword.|
|struct-to-int|Change a struct type to int type.|
|swap-casevalue|Swap two CaseStmt's values.|
|swap-child-expr|Randomly swap two child expressions of a BinaryOperator.|
|swap-expr|Swap two expressions if they are type compliant.|
|switch-to-goto|Convert a switch statement to goto statements.|
|switch-to-if|Convert a switch statement to a series of if-else statements.|
|toggle-bitfield|Toggle bitfield of a struct or union field.|
|toggle-break-continue|Change break to continue or vice versa.|
