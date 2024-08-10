public int foo(args>> int: y, z; char: ch): static {
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
        var int: pp<-foo(8,9, 's');
        var int: w<-9;
        x<-7;
        foo(w,x, 'c'); 
        
    }  
    else{
        x<-foo_2();
    }
    return x; 
}
private void foo_4() { var int: a; a <- 2; }


public void main(): static{
    var int: y,z; 
    var int: o <-foo(5,9, 'c');
    y <-foo(5,9, 'c');
    y<-8;
    z<-y;

}
