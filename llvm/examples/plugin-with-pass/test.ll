; test.ll - тестовый файл с циклами
define void @test_function(i32 %n) {
entry:
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  store i32 0, i32* %i, align 4
  br label %outer_loop

outer_loop:                                        ; preds = %outer_inc, %entry
  %0 = load i32, i32* %i, align 4
  %cmp1 = icmp slt i32 %0, %n
  br i1 %cmp1, label %outer_body, label %outer_end

outer_body:                                        ; preds = %outer_loop
  store i32 0, i32* %j, align 4
  br label %inner_loop

inner_loop:                                        ; preds = %inner_inc, %outer_body
  %1 = load i32, i32* %j, align 4
  %cmp2 = icmp slt i32 %1, %n
  br i1 %cmp2, label %inner_body, label %inner_end

inner_body:                                        ; preds = %inner_loop
  ; тело внутреннего цикла
  br label %inner_inc

inner_inc:                                         ; preds = %inner_body
  %2 = load i32, i32* %j, align 4
  %inc1 = add nsw i32 %2, 1
  store i32 %inc1, i32* %j, align 4
  br label %inner_loop

inner_end:                                         ; preds = %inner_loop
  br label %outer_inc

outer_inc:                                         ; preds = %inner_end
  %3 = load i32, i32* %i, align 4
  %inc2 = add nsw i32 %3, 1
  store i32 %inc2, i32* %i, align 4
  br label %outer_loop

outer_end:                                         ; preds = %outer_loop
  ret void
}

define void @simple_loop(i32 %n) {
entry:
  %i = alloca i32, align 4
  store i32 0, i32* %i, align 4
  br label %loop

loop:                                              ; preds = %inc, %entry
  %0 = load i32, i32* %i, align 4
  %cmp = icmp slt i32 %0, %n
  br i1 %cmp, label %body, label %end

body:                                              ; preds = %loop
  ; тело цикла
  br label %inc

inc:                                               ; preds = %body
  %1 = load i32, i32* %i, align 4
  %inc_simple = add nsw i32 %1, 1
  store i32 %inc_simple, i32* %i, align 4
  br label %loop

end:                                               ; preds = %loop
  ret void
}

; Функции, которые будут вызываться
declare void @loop_start()
declare void @loop_end()