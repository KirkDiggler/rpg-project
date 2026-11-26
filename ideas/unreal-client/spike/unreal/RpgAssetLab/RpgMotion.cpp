#include "RpgMotion.h"

#include "CoreMinimal.h"

#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"

namespace
{
    /// A cell as the record writes it: {"x": q, "y": r}.
    bool ReadCell(const FJsonObject& object, const TCHAR* key, FIntPoint* out)
    {
        const TSharedPtr<FJsonObject>* cell = nullptr;
        if (!object.TryGetObjectField(key, cell) || !cell->IsValid())
        {
            return false;
        }
        double q = 0.0;
        double r = 0.0;
        (*cell)->TryGetNumberField(TEXT("x"), q);
        (*cell)->TryGetNumberField(TEXT("y"), r);
        *out = FIntPoint(static_cast<int32>(q), static_cast<int32>(r));
        return true;
    }
} // namespace

bool PlanMotion(const FString& beat, const FString& fields_json, TMap<FString, FIntPoint>& cells,
                TArray<FMotionStep>& out)
{
    TSharedPtr<FJsonObject> fields;
    if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(fields_json), fields) || !fields.IsValid())
    {
        return false;
    }

    if (beat == TEXT("joined_at"))
    {
        // {"beat":"joined_at","cell":{"x":2,"y":0},"member":"guard-1"}: where a figure began.
        FString member;
        FIntPoint at;
        if (fields->TryGetStringField(TEXT("member"), member) && ReadCell(*fields, TEXT("cell"), &at))
        {
            cells.Add(member, at);
        }
        return false;
    }

    if (beat == TEXT("moved"))
    {
        // {"beat":"moved","member":"ue-walker","position":{"x":2,"y":1}}: one step, the cell arrived at.
        FString member;
        FIntPoint to;
        if (!fields->TryGetStringField(TEXT("member"), member) || !ReadCell(*fields, TEXT("position"), &to))
        {
            return false;
        }
        FMotionStep step;
        step.kind = FMotionStep::Kind::Walk;
        step.member = member;
        step.from = cells.FindRef(member);
        step.to = to;
        cells.Add(member, to);
        out.Add(step);
        return true;
    }

    if (beat == TEXT("missed") || beat == TEXT("struck"))
    {
        // {"actor":"ue-dummy","beat":"missed","roll":1,"targets":["ue-fighter"],...}: a swing, either way.
        FString actor;
        const TArray<TSharedPtr<FJsonValue>>* targets = nullptr;
        if (!fields->TryGetStringField(TEXT("actor"), actor) || !fields->TryGetArrayField(TEXT("targets"), targets) ||
            targets->Num() == 0)
        {
            return false;
        }
        FMotionStep step;
        step.kind = FMotionStep::Kind::Swing;
        step.member = actor;
        (*targets)[0]->TryGetString(step.target);
        step.from = cells.FindRef(actor);
        step.to = cells.FindRef(step.target);
        out.Add(step);
        return true;
    }

    if (beat == TEXT("turn-ended"))
    {
        // {"beat":"turn-ended","member":"ue-fighter","next":"ue-dummy"}: whose turn it now is.
        FString next;
        if (!fields->TryGetStringField(TEXT("next"), next) || next.IsEmpty())
        {
            return false;
        }
        FMotionStep step;
        step.kind = FMotionStep::Kind::Turn;
        step.member = next;
        step.to = cells.FindRef(next);
        out.Add(step);
        return true;
    }

    return false;
}

namespace
{
    constexpr double kSqrt3 = 1.7320508075688772;
    constexpr double kCmPerCell = 100.0;

    /// The cell a point of the cube grid rounds to: round each axis, then put
    /// the one that moved furthest back on the cube.
    FIntPoint RoundedCell(double x, double y, double z)
    {
        double rx = FMath::RoundToDouble(x);
        double ry = FMath::RoundToDouble(y);
        double rz = FMath::RoundToDouble(z);
        const double dx = FMath::Abs(rx - x);
        const double dy = FMath::Abs(ry - y);
        const double dz = FMath::Abs(rz - z);
        if (dx > dy && dx > dz)
        {
            rx = -ry - rz;
        }
        else if (dy > dz)
        {
            ry = -rx - rz;
        }
        else
        {
            rz = -rx - ry;
        }
        return FIntPoint(static_cast<int32>(rx), static_cast<int32>(rz));
    }
} // namespace

FIntPoint CellAt(const FVector2D& centimetres)
{
    // The inverse of the layout: r is y over 150; q is x over the column
    // spacing, less half of r.
    const double r = centimetres.Y / (1.5 * kCmPerCell);
    const double q = centimetres.X / (kSqrt3 * kCmPerCell) - r / 2.0;
    return RoundedCell(q, -q - r, r);
}

TArray<FIntPoint> HexLine(const FIntPoint& from, const FIntPoint& to)
{
    // Cube coordinates: x is q, z is r, and y closes the cube at -x - z. The
    // distance between two cells is the largest of the three differences.
    struct FCube
    {
        double x;
        double y;
        double z;
    };
    const auto cube = [](const FIntPoint& hex) {
        return FCube{static_cast<double>(hex.X), -static_cast<double>(hex.X) - static_cast<double>(hex.Y),
            static_cast<double>(hex.Y)};
    };
    const FCube a = cube(from);
    const FCube b = cube(to);
    const double distance = FMath::Max3(FMath::Abs(a.x - b.x), FMath::Abs(a.y - b.y), FMath::Abs(a.z - b.z));
    const int32 steps = static_cast<int32>(FMath::RoundToDouble(distance));

    TArray<FIntPoint> line;
    for (int32 i = 0; i <= steps; ++i)
    {
        const double t = steps == 0 ? 0.0 : static_cast<double>(i) / static_cast<double>(steps);
        line.Add(RoundedCell(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t));
    }
    return line;
}
