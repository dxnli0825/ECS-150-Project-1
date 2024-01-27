# SSHELL: Simple Shell

## Summary

The program `sshell` is a simple form of the operating system shell that 
can perform similar functions of the operating system shell. The program is
able to perform file manipulation, command line arguments, and some other 
sample build-in functions.

## Implementation

The implementation of the program `sshell` follows three main different procedures:

1. Run the program and read the input from the user.
2. Based on the user input, perform the specified task.
3. Provide feedback to the user, on whether the command they have entered is 
    invalid or the command has been successfully executed. 

## Testing

We did a lot of tests for each phase by using the tester.sh which is provided in class. The most important thing is that we need to test it through our CSIF environment and by using the printf function every time we faced an error, we were able to passed the first 3 test cases. By following each phase, we kept improving our code, and then slowly passed more test cases, incleasing the score by 0.5 each time. But unfortunately, we were only able to passed 5 test cases in total until we reach the time limit. Even though the output of out program, using the examples in the `project.html`, seems matched, the test result was not looks good. Even though we did a hardwork on the code and work until the last minutes, we still cannot firgure it out. Since we are out of time, so we decided to leave it there. 

## ChalLenges

Obviously, it is a very challenge project. The first challenge is that we both are not familiar with the C language. We did not know what the c library contains as we learn strtok() and strchr() was right before the date line. Moreover, the most difficult part of this project is the topic of piping commands. Probably, we didn't do well in this part because we didn't know how to perform fork in the loop. Lastly, we believe the result of the test case is not very clear, where we are only able to know it didn't pass the test case, but we didn't know how to perform fork in the loop. Lastly, we believe the result of the test case is not very clear, where we are only able to know it didn't pass the test case, but we didn't know, or we cannot figure out why it failed.

## Reference

We used the Markdown Cheatsheet that is provided in the project instruction, we also used the Linux man pages to find out the function that we used in thisproject. And also, we did some online research to find out some online resourceto help us better understand the C language and even fix the syntax error.

Linux man pages: https://linux.die.net/man/

Markdown Cheatsheet: https://github.com/adam-p/markdown-here/wikiMarkdown-Cheatsheet

Cplusplus: https://cplusplus.com/reference/cstring/strtok/#google _vignette
