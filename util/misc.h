#pragma once

struct RankedScore
{
    void* scoree;
    int scoreCount;
    i32 score[16];
};

function RankedScore
GetBestScore(RankedScore incumbent, RankedScore challenger)
{
    Assert(challenger.scoree);

    RankedScore result;
    if (!incumbent.scoree)
    {
        result = challenger;
    }
    else
    {
        Assert(incumbent.scoreCount == challenger.scoreCount);
        Assert(challenger.scoreCount < ArrayLen(challenger.score));

        result = incumbent;

        for (int iScore = 0; iScore < ArrayLen(challenger.score); iScore++)
        {
            // TODO - Option to prefer high scores or low scores. Expose this option per score?

            if (challenger.score[iScore] < incumbent.score[iScore])
            {
                result = challenger;
                break;
            }
            else if (challenger.score[iScore] > incumbent.score[iScore])
            {
                result = incumbent;
                break;
            }
            else
            {
                // Tied. Go to the next score!
            }
        }
    }

    return result;
}

// --- Less sucky version of the C++ "pointer-to-member" feature

// @Style - Prefix memberRef variables with m

template <typename TStruct, typename TMember>
struct MemberRef
{
    int byteOffset;
    bool isValid; // NOTE - Useful for ZII, Since 0 byte offset is equivalent to the first member. Worth the extra memory/branching.

    MemberRef() = default;
    MemberRef(int byteOffset)
    {
        this->byteOffset = byteOffset;
        if (byteOffset >= 0)
        {
            this->isValid = true;
        }
    }
};

#define MakeMemberRef(TYPE, MEMBER_TYPE, MEMBER_NAME) (MemberRef<TYPE, MEMBER_TYPE>(offsetof(TYPE, MEMBER_NAME)))

template <typename TStruct, typename TMember>
inline MemberRef<TStruct, TMember>
MemberRef_Nil()
{
    MemberRef<TStruct, TMember> result = {};
    return result;
}

template <typename TStruct, typename TMember>
inline TMember*
PMember(TStruct* pStruct, MemberRef<TStruct, TMember> memberRef)
{
    TMember * result = nullptr;
    if (memberRef.isValid)
    {
        result = (TMember *)((u8 *)pStruct + memberRef.byteOffset);
    }

    return result;
}
