; ModuleID = 'cminus'
source_filename = "../tests/4-ir-opt/testcases/GVN/functional/bin.cminus"

declare i32 @input()

declare void @output(i32)

declare void @outputFloat(float)

declare void @neg_idx_except()

define i32 @main() {
label_entry:
  %op0 = alloca i32
  %op1 = alloca i32
  %op2 = alloca i32
  %op3 = alloca i32
  %op4 = call i32 @input()
  %op5 = call i32 @input()
  %op6 = icmp sgt i32 %op4, %op5
  %op7 = zext i1 %op6 to i32
  %op8 = icmp ne i32 %op7, 0
  br i1 %op8, label %label9, label %label25
label9:                                                ; preds = %label_entry
  %op10 = load i32, i32* %op0
  %op11 = add i32 33, 33
  store i32 %op11, i32* %op0
  %op12 = load i32, i32* %op1
  %op13 = add i32 44, 44
  store i32 %op13, i32* %op1
  %op14 = load i32, i32* %op2
  %op15 = load i32, i32* %op1
  %op16 = load i32, i32* %op0
  %op17 = add i32 %op16, %op15
  store i32 %op17, i32* %op2
  br label %label18
label18:                                                ; preds = %label9, %label25
  %op19 = load i32, i32* %op2
  call void @output(i32 %op19)
  %op20 = load i32, i32* %op3
  %op21 = load i32, i32* %op1
  %op22 = load i32, i32* %op0
  %op23 = add i32 %op22, %op21
  store i32 %op23, i32* %op3
  %op24 = load i32, i32* %op3
  call void @output(i32 %op24)
  ret i32 0
label25:                                                ; preds = %label_entry
  %op26 = load i32, i32* %op0
  %op27 = add i32 55, 55
  store i32 %op27, i32* %op0
  %op28 = load i32, i32* %op1
  %op29 = add i32 66, 66
  store i32 %op29, i32* %op1
  %op30 = load i32, i32* %op2
  %op31 = load i32, i32* %op1
  %op32 = load i32, i32* %op0
  %op33 = add i32 %op32, %op31
  store i32 %op33, i32* %op2
  br label %label18
}
