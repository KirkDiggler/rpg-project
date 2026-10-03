package encounter

import (
	"encoding/json"
	"github.com/KirkDiggler/rpg-toolkit/mind/perception"
	"github.com/KirkDiggler/rpg-toolkit/tools/spatial"
	"github.com/stretchr/testify/require"
	"os"
	"testing"
)

// R&D only. Reuse existing geometry test capabilities, with a stated 24-cell
// range matching the session fallback; no production sheet/light model.
type rd508Sight struct{}

func (rd508Sight) Sight(ids []MemberID) (map[MemberID]int, error) {
	out := map[MemberID]int{}
	for _, id := range ids {
		out[id] = 24
	}
	return out, nil
}

type rd508Checks struct{}

func (rd508Checks) ResolveCheck(*ResolveCheckInput) (*ResolveCheckOutput, error) {
	return &ResolveCheckOutput{Beaten: false}, nil
}

type rd508Witness struct{}

func (rd508Witness) Perceivers(*PerceiversInput) ([]MemberID, error) { return nil, nil }
func rd508Load(t *testing.T, data EncounterData) *Encounter {
	t.Helper()
	e, err := LoadEncounter(&LoadEncounterInput{Data: data,
		Sight: rd508Sight{}, Equipment: noHandsAreObserved{}, Standing: everyoneStanding{}, Initiative: orderAsGiven{},
		TurnDriver: passDriver{}, Striker: passStriker{}, Mover: quietMover{}, Announcer: quietAnnouncer{}, CheckResolver: rd508Checks{}, Witness: rd508Witness{},
	})
	require.NoError(t, err)
	return e
}
func rd508Tomb(t *testing.T) *Encounter {
	t.Helper()
	raw, err := os.ReadFile(os.Getenv("RD508_DATA"))
	require.NoError(t, err)
	var data EncounterData
	require.NoError(t, json.Unmarshal(raw, &data))
	return rd508Load(t, data)
}

// Ask the EXISTING sightReach at a supplied target cell. This is an exact
// range/LOS/runtime-obstruction query, NOT a new production absence producer.
// "probe-target" is not inserted into the roster or perception store.
func rd508Reaches(e *Encounter, from, to spatial.Position) bool {
	return (sightReach{positions: map[MemberID]spatial.Position{"alice": from, "probe-target": to}, cells: map[MemberID]int{"alice": 24}, canvas: e.canvas, areas: e.sightAreas}).Reaches(perception.Sight, "alice", "probe-target")
}
func TestRD508SpatialScan(t *testing.T) {
	e := rd508Tomb(t)
	door := e.doorsByID["reference-tomb-heirloom/vault"]
	require.NotNil(t, door)
	prop := e.field.cellAt(e.field.props[e.field.propIndexOf("heirloom")].At)
	t.Logf("heirloom absolute=%v; door endpoints=%v", prop, door.edges)
	counts := map[string]int{}
	examples := map[string][][2]int{}
	// Scan authored cells, keep legitimate standable floor; actual movement is
	// checked separately. Door endpoints are candidate observation supports,
	// not a settled claim that seeing an endpoint sees the whole door.
	for row := 0; row < 8; row++ {
		for col := 0; col < 30; col++ {
			cell := e.field.cellAt(spatial.Position{X: float64(col), Y: float64(row)})
			if !e.field.isStandable(cell) {
				continue
			}
			fact, err := e.CellAt(CellAtInput{Mover: "alice", Cell: cell})
			require.NoError(t, err)
			if fact.Passage != PassageStandable {
				continue
			}
			p := rd508Reaches(e, cell, prop)
			a := rd508Reaches(e, cell, door.edges[0].From)
			b := rd508Reaches(e, cell, door.edges[0].To)
			key := "other"
			if p && (a || b) {
				key = "open-prop-and-door"
			}
			if !p && !a && !b {
				key = "open-neither"
			}
			if !p && (a || b) {
				key = "open-door-not-prop"
			}
			if key == "open-neither" && col >= 16 {
				t.Logf("tomb blind authored=[%d,%d] axial=%v", col, row, cell)
			}
			counts[key]++
			if len(examples[key]) < 12 {
				examples[key] = append(examples[key], [2]int{col, row})
			}
		}
	}
	t.Logf("open counts=%v examples authored=%v", counts, examples)
	_, err := e.CloseDoor(&CloseDoorInput{Actor: "bob", Door: door.id})
	require.NoError(t, err)
	counts = map[string]int{}
	examples = map[string][][2]int{}
	for row := 0; row < 8; row++ {
		for col := 0; col < 30; col++ {
			cell := e.field.cellAt(spatial.Position{X: float64(col), Y: float64(row)})
			if !e.field.isStandable(cell) {
				continue
			}
			fact, err := e.CellAt(CellAtInput{Mover: "alice", Cell: cell})
			require.NoError(t, err)
			if fact.Passage != PassageStandable {
				continue
			}
			p := rd508Reaches(e, cell, prop)
			a := rd508Reaches(e, cell, door.edges[0].From)
			b := rd508Reaches(e, cell, door.edges[0].To)
			key := "other"
			if !p && (a || b) {
				key = "closed-door-not-prop"
			}
			if p {
				key = "closed-prop"
			}
			counts[key]++
			if len(examples[key]) < 12 {
				examples[key] = append(examples[key], [2]int{col, row})
			}
		}
	}
	t.Logf("closed counts=%v examples authored=%v", counts, examples)
	// Candidate start ranges and obscuring effects use the same reach code.
	from := e.field.cellAt(spatial.Position{X: 28, Y: 2})
	require.True(t, rd508Reaches(e, from, prop))
	require.False(t, (sightReach{positions: map[MemberID]spatial.Position{"alice": from, "probe-target": prop}, cells: map[MemberID]int{"alice": 0}, canvas: e.canvas, areas: e.sightAreas}).Reaches(perception.Sight, "alice", "probe-target"), "absence evidence must respect supplied range")
	require.NoError(t, e.AddSightArea(&SightAreaInput{ID: "probe-fog", SourceID: "probe", Center: prop, RadiusFeet: 5}))
	require.False(t, rd508Reaches(e, from, prop), "runtime sight obstruction applies to empty-position evidence too")
}
