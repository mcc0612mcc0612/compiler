; ModuleID = 'cminus'
source_filename = "../tests/3-ir-gen/testcases/lv1/assign_cmp.cminus"

declare i32 @input()

declare void @output(i32)

declare void @outputFloat(float)

declare void @neg_idx_except()

define void @main() {
label_entry:
  %op0 = alloca i32
  %op1 = alloca i32
  %op2 = alloca i32
  %op3 = load i32, i32* %op0
  %op4 = icmp slt i32 1, 3
  %op5 = zext i1 %op4 to i32
  store i32 %op5, i32* %op0
  %op6 = load i32, i32* %op1
  %op7 = icmp eq i32 2, 4
  %op8 = zext i1 %op7 to i32
  store i32 %op8, i32* %op1
  %op9 = load i32, i32* %op2
  %op10 = icmp sgt i32 3, 5
  %op11 = zext i1 %op10 to i32
  store i32 %op11, i32* %op2
  %op12 = load i32, i32* %op0
  call void @output(i32 %op12)
  %op13 = load i32, i32* %op1
  call void @output(i32 %op13)
  %op14 = load i32, i32* %op2
  call void @output(i32 %op14)
  ret void
}
