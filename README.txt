מגישים:
עידן וייס - 214573842
ריף יוסף מיזן - 214853137
אנדריי יוטיש - 336405527


fixes:
in yacc file: 
added support for: {} blocks that wasnt there before 
added support for ^ assignment
fixed in type literals string_literal
in semantic:
changed assignment to support ^
changed so it creates scope for {} block
fixed missing |length| in rule 16
fixed rules 13, 14 in get type