main(argc, argv)
    int pipefd[2]
    return
        pipeTag
        [pipefd] 
        pr1(pipeResult)
            return
                forkTag
                []
                pr2(forkResult)
                    if (forkResult)
                        return
                            dup2Tag
                            [pipefd[1], 1]
                            pr3(dup2Result)
                                return
                                    closeTag
                                    [pipefd[0]]
                                    pr4(closeResult)
                                        return
                                            closeTag
                                            [pipefd[1]]
                                            pr5(closeResult)
                                                return
                                                    execlTag
                                                    ["/bin/ls/", "ls", NULL]
                                                    pr6(execlResult)
                                                        return
                                                            exitTag
                                                            []
                                                            NULL
                    else
                        return
                            dup2Tag
                            [pipefd[1], 0]
                            pr7(dup2Result)
                                return
                                    closeTag
                                    [pipefd[0]]
                                    pr8(closeResult)
                                        return
                                            closeTag
                                            [pipefd[1]]
                                            pr9(closeResult)
                                                return
                                                    execlTag
                                                    ["/bin/grep", "grep", NULL]
                                                    pr10(execlResult)
                                                        return
                                                            exitTag
                                                            []
                                                            NULL
