int fact(int n)
{
	int temp;
    if(n==1)
    {
        return n;
    }
    else
    {
    	temp=(n*fact(n-1));
        return temp;
    }
}


int main()
{
    int i;
    int result,times;
    times=read();
    for(i=0; i<times; i=i+1)
    {
    	int m = read();
    	if( m > 1)
        {
        	result=fact(m);
        }
        else
        {
            result = 1;
        }
        print(result);
    }
    return 0;
}
