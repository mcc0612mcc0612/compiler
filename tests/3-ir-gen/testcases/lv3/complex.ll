; ModuleID = 'complex.c'
source_filename = "complex.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@x = common dso_local global [10 x float] zeroinitializer, align 16

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @minloc(float* %0, float %1, i32 %2) #0 {
  %4 = alloca float*, align 8
  %5 = alloca float, align 4
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  %8 = alloca i32, align 4
  %9 = alloca i32, align 4
  store float* %0, float** %4, align 8
  store float %1, float* %5, align 4
  store i32 %2, i32* %6, align 4
  %10 = load float, float* %5, align 4
  %11 = fptosi float %10 to i32
  store i32 %11, i32* %9, align 4
  %12 = load float*, float** %4, align 8
  %13 = getelementptr inbounds float, float* %12, i64 10
  %14 = load float, float* %13, align 4
  %15 = fptosi float %14 to i32
  store i32 %15, i32* %8, align 4
  %16 = load float, float* %5, align 4
  %17 = fadd float %16, 1.000000e+00
  %18 = fptosi float %17 to i32
  store i32 %18, i32* %7, align 4
  br label %19

19:                                               ; preds = %40, %3
  %20 = load i32, i32* %7, align 4
  %21 = load i32, i32* %6, align 4
  %22 = icmp slt i32 %20, %21
  br i1 %22, label %23, label %43

23:                                               ; preds = %19
  %24 = load float*, float** %4, align 8
  %25 = load i32, i32* %7, align 4
  %26 = sext i32 %25 to i64
  %27 = getelementptr inbounds float, float* %24, i64 %26
  %28 = load float, float* %27, align 4
  %29 = load i32, i32* %8, align 4
  %30 = sitofp i32 %29 to float
  %31 = fcmp olt float %28, %30
  br i1 %31, label %32, label %40

32:                                               ; preds = %23
  %33 = load float*, float** %4, align 8
  %34 = load i32, i32* %7, align 4
  %35 = sext i32 %34 to i64
  %36 = getelementptr inbounds float, float* %33, i64 %35
  %37 = load float, float* %36, align 4
  %38 = fptosi float %37 to i32
  store i32 %38, i32* %8, align 4
  %39 = load i32, i32* %7, align 4
  store i32 %39, i32* %9, align 4
  br label %40

40:                                               ; preds = %32, %23
  %41 = load i32, i32* %7, align 4
  %42 = add nsw i32 %41, 1
  store i32 %42, i32* %7, align 4
  br label %19

43:                                               ; preds = %19
  %44 = load i32, i32* %9, align 4
  ret i32 %44
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @sort(float* %0, i32 %1, float %2) #0 {
  %4 = alloca float*, align 8
  %5 = alloca i32, align 4
  %6 = alloca float, align 4
  %7 = alloca i32, align 4
  %8 = alloca i32, align 4
  %9 = alloca i32, align 4
  store float* %0, float** %4, align 8
  store i32 %1, i32* %5, align 4
  store float %2, float* %6, align 4
  %10 = load i32, i32* %5, align 4
  store i32 %10, i32* %7, align 4
  br label %11

11:                                               ; preds = %17, %3
  %12 = load i32, i32* %7, align 4
  %13 = sitofp i32 %12 to float
  %14 = load float, float* %6, align 4
  %15 = fsub float %14, 1.000000e+00
  %16 = fcmp olt float %13, %15
  br i1 %16, label %17, label %47

17:                                               ; preds = %11
  %18 = load float*, float** %4, align 8
  %19 = load i32, i32* %7, align 4
  %20 = sitofp i32 %19 to float
  %21 = load float, float* %6, align 4
  %22 = fptosi float %21 to i32
  %23 = call i32 @minloc(float* %18, float %20, i32 %22)
  store i32 %23, i32* %8, align 4
  %24 = load float*, float** %4, align 8
  %25 = load i32, i32* %8, align 4
  %26 = sext i32 %25 to i64
  %27 = getelementptr inbounds float, float* %24, i64 %26
  %28 = load float, float* %27, align 4
  %29 = fptosi float %28 to i32
  store i32 %29, i32* %9, align 4
  %30 = load float*, float** %4, align 8
  %31 = load i32, i32* %7, align 4
  %32 = sext i32 %31 to i64
  %33 = getelementptr inbounds float, float* %30, i64 %32
  %34 = load float, float* %33, align 4
  %35 = load float*, float** %4, align 8
  %36 = load i32, i32* %8, align 4
  %37 = sext i32 %36 to i64
  %38 = getelementptr inbounds float, float* %35, i64 %37
  store float %34, float* %38, align 4
  %39 = load i32, i32* %9, align 4
  %40 = sitofp i32 %39 to float
  %41 = load float*, float** %4, align 8
  %42 = load i32, i32* %7, align 4
  %43 = sext i32 %42 to i64
  %44 = getelementptr inbounds float, float* %41, i64 %43
  store float %40, float* %44, align 4
  %45 = load i32, i32* %7, align 4
  %46 = add nsw i32 %45, 1
  store i32 %46, i32* %7, align 4
  br label %11

47:                                               ; preds = %11
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  store i32 0, i32* %2, align 4
  br label %3

3:                                                ; preds = %6, %0
  %4 = load i32, i32* %2, align 4
  %5 = icmp slt i32 %4, 10
  br i1 %5, label %6, label %14

6:                                                ; preds = %3
  %7 = call i32 (...) @input()
  %8 = sitofp i32 %7 to float
  %9 = load i32, i32* %2, align 4
  %10 = sext i32 %9 to i64
  %11 = getelementptr inbounds [10 x float], [10 x float]* @x, i64 0, i64 %10
  store float %8, float* %11, align 4
  %12 = load i32, i32* %2, align 4
  %13 = add nsw i32 %12, 1
  store i32 %13, i32* %2, align 4
  br label %3

14:                                               ; preds = %3
  call void @sort(float* getelementptr inbounds ([10 x float], [10 x float]* @x, i64 0, i64 0), i32 0, float 1.000000e+01)
  store i32 0, i32* %2, align 4
  br label %15

15:                                               ; preds = %18, %14
  %16 = load i32, i32* %2, align 4
  %17 = icmp slt i32 %16, 10
  br i1 %17, label %18, label %27

18:                                               ; preds = %15
  %19 = load i32, i32* %2, align 4
  %20 = sext i32 %19 to i64
  %21 = getelementptr inbounds [10 x float], [10 x float]* @x, i64 0, i64 %20
  %22 = load float, float* %21, align 4
  %23 = fpext float %22 to double
  %24 = call i32 (double, ...) bitcast (i32 (...)* @output to i32 (double, ...)*)(double %23)
  %25 = load i32, i32* %2, align 4
  %26 = add nsw i32 %25, 1
  store i32 %26, i32* %2, align 4
  br label %15

27:                                               ; preds = %15
  ret i32 0
}

declare dso_local i32 @input(...) #1

declare dso_local i32 @output(...) #1

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.1 "}
