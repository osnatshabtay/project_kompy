public int foo(args>> int: y, z): static {
     return 0; 
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
        var int: pp<-foo(8,9);
        var int: w<-9;
        x<-7;
        foo(w,x); 
        
    }  
    else{
        x<-foo_2();
    }
    return x; 
}
private void foo_4() { var int: a; a <- 2; }


public void main(): static{
    var int: y,z; 
    var int: o <-foo(5,9);
    y<-8;
    z<-y;

}
