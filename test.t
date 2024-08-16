public int foo(args>> int: y, z; char: ch): static {
     string a[30] <- "moshe";
     return |a|; 
    }

public int foo_5(): static { 
    var int: a;
    var int: x; 
    a <- 2;
    return 2; 
    }

public int foo_2(){ 
    var int: a;
    var int: x; 
    a <- 2; 
    return a+9; }
private int foo_3() { 
    var int: x<-5;
    if (true) 
    { 
        var int: pp<-foo(8,9, 's');
        var int: w<-9;
        w<-9;
        /* x<-7;*/
        foo(w,x, 'c');
        
    }  
    else{
        x<-foo_2();
    }
    return x; 
}
private void foo_4() { var int: a; a <- 2; }


public void main(): static{
    var double: d;
    var bool: b;
    var int: y,z; 
    var int: o <-foo(5,9, 'c');
    y <-foo(5,9, 'c');
    y<-8+4;
    z<-y;
    d<-1.1+2;
    b<-d<3.3;
    b<-y==z;
    foo_5();
}
