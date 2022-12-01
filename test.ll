@globVar = global i32 zeroinitializer
declare void @neg_idx_except()
define i32 @func(i32 %arg0) {
label_entry:
  %op1 = alloca i32
  store i32 %arg0, i32* %op1
  %op2 = load i32, i32* %op1
  %op3 = icmp sgt i32 %op2, 0
  %op4 = zext i1 %op3 to i32
  %op5 = icmp ne i32 %op4, 0
  br i1 %op5, label %label6, label %label7
label6:                                                ; preds = %label_entry
  store i32 0, i32* %op1
  br label %label7
label7:                                                ; preds = %label_entry, %label6
  %op8 = load i32, i32* %op1
  ret i32 %op8
}
define i32 @main() {
label_entry:
  %op0 = alloca [10 x i32]
  %op1 = alloca i32
  store i32 1, i32* @globVar
  %op2 = icmp slt i32 5, 0
  br i1 %op2, label %label3, label %label4
label3:                                                ; preds = %label_entry
  call void @neg_idx_except()
  ret i32 0
label4:                                                ; preds = %label_entry
  %op5 = getelementptr [10 x i32], [10 x i32]* %op0, i32 0, i32 5
  store i32 999, i32* %op5
  store i32 2333, i32* %op1
  %op6 = load i32, i32* %op1
  %op7 = call i32 @func(i32 %op6)
  %op8 = load i32, i32* @globVar
  %op9 = call i32 @func(i32 %op8)
  ret i32 0
}