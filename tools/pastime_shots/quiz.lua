-- Battle quiz hall, with two players online (run.py ... 2): TEST1 plays a
-- round in Mauville (one wrong answer on purpose), wins today's prize and
-- opens R > PASTIMES > QUIZ HALL and the QUIZ CHAMPS board. TEST2 waits.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
local P1 = INSTANCE == 1

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    if not P1 then
        D.scene(12, MAPS.MAP_MAUVILLE_CITY, 18, 12)
        D.wait(6000)
        return
    end
    D.scene(12, MAPS.MAP_MAUVILLE_CITY, 21, 11)
    D.wait(60)
    D.shot("quizmaster")
    D.talk(K.UP, "hello", 2)
    D.press(K.A, 60) -- the rules
    D.press(K.A, 60)
    for q = 1, 10 do
        D.waitFor("question " .. q, 600, function() return D.read8(D.SYM.gQuizRound) == q end, function(i)
            if i % 60 == 0 then D.press(K.A, 0) end
        end)
        D.wait(120)
        if q <= 2 or q == 4 then D.shot("question-" .. q) end
        local answer = D.read8(D.SYM.gQuizRound + 3)
        if q == 4 then answer = (answer + 1) % 4 end
        for _ = 1, answer do D.press(K.DOWN, 8) end
        if q == 1 then D.shot("answer-" .. q) end
        D.press(K.A, 60)
        if q <= 2 or q == 4 or q == 9 then D.shot("reply-" .. q) end
        D.press(K.A, 30)
    end
    D.wait(200)
    D.shot("round-over")
    D.press(K.A, 60)
    D.shot("round-over-2")
    D.press(K.A, 60)
    D.shot("prize")
    D.press(K.A, 200)
    D.shot("prize-item")
    for _ = 1, 5 do D.press(K.B, 40) end
    D.wait(60)
    D.pastimeMenu(4)
    D.wait(20)
    D.press(K.A, 40)
    D.wait(20)
    D.shot("records")
    D.press(K.B, 60)
    D.wait(30)
    D.pastimeMenu(4)
    D.wait(20)
    D.press(K.DOWN, 10)
    D.press(K.A, 40)
    D.wait(120)
    D.shot("quiz-champs")
    D.press(K.B, 40)
end)
