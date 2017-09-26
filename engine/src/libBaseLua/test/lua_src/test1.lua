function test_function1(a, b)
    return a + b
end

function test_function2(pAA)
    pAA.a = 222
    return pAA
end

function test_function3(pAA)
    pAA:funA(333)
    return pAA
end

function test_function4(pBB)
    pBB:funA(444)
    return pBB
end

function test_function5(pBB)
    pBB:funB(555)
    return pBB
end

function test_function6()
    local pBB = cc.CBB.new(0, 0)
    pBB.a = 666
    return pBB
end

function test_function7()
    local pBB = cc.getBB()
    pBB.b = cc.nBB
    return pBB
end