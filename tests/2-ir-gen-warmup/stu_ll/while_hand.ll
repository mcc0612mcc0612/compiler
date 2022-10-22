; ModuleID = '../c_cases/while.c'
source_filename = "../c_cases/while.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @main() {
label_entry:
  %op0 = alloca i32
  %op1 = alloca i32
  %op2 = getelementptr i32, i32* %op0, i32 0
  store i32 10, i32* %op2
  %op3 = getelementptr i32, i32* %op1, i32 0
  store i32 0, i32* %op3
  br label %label_haha
label_haha:                                                ; preds = %label_entry, %label_trueBB
  %op4 = load i32, i32* %op3
  %op5 = icmp slt i32 %op4, 10
  br i1 %op5, label %label_trueBB, label %label_falseBB
label_trueBB:                                                ; preds = %label_haha
  %op6 = load i32, i32* %op1
  %op7 = add i32 1, %op6
  store i32 %op7, i32* %op1
  %op8 = load i32, i32* %op1
  %op9 = load i32, i32* %op0
  %op10 = add i32 %op8, %op9
  store i32 %op10, i32* %op0
  br label %label_haha
label_falseBB:                                                ; preds = %label_haha
  %op11 = load i32, i32* %op2
  ret i32 %op11
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.1 "}
