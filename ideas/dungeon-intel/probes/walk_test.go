package encounter

import (
	"encoding/json"
	"github.com/KirkDiggler/rpg-toolkit/tools/spatial"
	"github.com/stretchr/testify/require"
	"testing"
)

func rd508Walk(t *testing.T, e *Encounter, authored spatial.Position) {
	t.Helper()
	from, ok := e.canvas.GetEntityPosition("alice")
	require.True(t, ok)
	target := e.field.cellAt(authored)
	path, reached, err := e.routeTo("alice", from, func(p spatial.Position) bool { return p == target })
	require.NoError(t, err)
	require.True(t, reached)
	t.Logf("A walk to authored %v / axial %v: %v", authored, target, path)
	for _, p := range path {
		_, err = e.Step(&StepInput{Member: "alice", To: p})
		require.NoError(t, err)
	}
}
func rd508HasHeirloom(t *testing.T, e *Encounter) bool {
	t.Helper()
	atlas, err := e.AtlasFor("alice")
	require.NoError(t, err)
	for _, p := range atlas.Props {
		if p.ID == "heirloom" {
			return true
		}
	}
	return false
}
func rd508DoorState(t *testing.T, e *Encounter) DoorStateKind {
	t.Helper()
	doors, err := e.DoorsFor("alice")
	require.NoError(t, err)
	for _, d := range doors {
		if d.ID == "reference-tomb-heirloom/vault" {
			return d.State.Kind()
		}
	}
	t.Fatal("A must know vault door from fixture setup")
	return ""
}
func rd508RoundTrip(t *testing.T, e *Encounter) *Encounter {
	t.Helper()
	raw, err := json.Marshal(e.ToData())
	require.NoError(t, err)
	var data EncounterData
	require.NoError(t, json.Unmarshal(raw, &data))
	return rd508Load(t, data)
}
func TestRD508TombWalkBaseline(t *testing.T) {
	e := rd508Tomb(t)
	door := e.doorsByID["reference-tomb-heirloom/vault"]
	prop := e.field.cellAt(e.field.props[e.field.propIndexOf("heirloom")].At)
	start, _ := e.canvas.GetEntityPosition("alice")
	require.True(t, rd508Reaches(e, start, prop))
	require.True(t, rd508HasHeirloom(t, e))
	require.Equal(t, DoorOpen, rd508DoorState(t, e))
	rd508Walk(t, e, spatial.Position{X: 27, Y: 3})
	withdrawn, _ := e.canvas.GetEntityPosition("alice")
	require.False(t, rd508Reaches(e, withdrawn, prop))
	for _, edge := range door.edges {
		require.False(t, rd508Reaches(e, withdrawn, edge.From))
		require.False(t, rd508Reaches(e, withdrawn, edge.To))
	}
	_, err := e.Hold(&HoldInput{Member: "bob", Target: "heirloom"})
	require.NoError(t, err)
	status, err := e.Status()
	require.NoError(t, err)
	require.Nil(t, status.Outcome, "pickup does not end the run")
	require.False(t, rd508HasHeirloom(t, e), "BASELINE GAP: blind A's atlas drops the heirloom immediately")
	_, err = e.CloseDoor(&CloseDoorInput{Actor: "bob", Door: door.id})
	require.NoError(t, err)
	require.Equal(t, DoorClosed, rd508DoorState(t, e), "BASELINE GAP: blind A reads the unseen new door state")
	e = rd508RoundTrip(t, e)
	require.False(t, rd508HasHeirloom(t, e))
	require.Equal(t, DoorClosed, rd508DoorState(t, e))
	entries, err := e.Story(&StoryInput{Audience: "alice"})
	require.NoError(t, err)
	seenHeld, seenClosed := false, false
	for _, entry := range entries {
		var beat map[string]any
		require.NoError(t, json.Unmarshal(entry.Payload, &beat))
		if beat["beat"] == "held" && beat["prop"] == "heirloom" {
			seenHeld = true
			t.Logf("BASELINE LEAK A story: %s", entry.Payload)
		}
		if beat["beat"] == "door" && beat["state"] == "closed" {
			seenClosed = true
			t.Logf("BASELINE LEAK A story: %s", entry.Payload)
		}
	}
	require.True(t, seenHeld)
	require.True(t, seenClosed)
	rd508Walk(t, e, spatial.Position{X: 27, Y: 5})
	near, _ := e.canvas.GetEntityPosition("alice")
	door = e.doorsByID[door.id]
	require.True(t, rd508Reaches(e, near, door.edges[0].From))
	require.False(t, rd508Reaches(e, near, prop))
	_, err = e.OpenDoor(&OpenDoorInput{Actor: "alice", Door: door.id})
	require.NoError(t, err)
	require.True(t, rd508Reaches(e, near, prop), "old position is observable without standing on it")
	require.NotEqual(t, near, prop)
	e = rd508RoundTrip(t, e)
	require.False(t, rd508HasHeirloom(t, e))
	status, err = e.Status()
	require.NoError(t, err)
	require.Nil(t, status.Outcome)
	t.Log("normal Step/Hold/CloseDoor/OpenDoor and two JSON reloads succeeded; current knowledge projection deliberately fails new acceptance behavior")
}
