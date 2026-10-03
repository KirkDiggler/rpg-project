package encounter

import (
	"encoding/json"
	"github.com/KirkDiggler/rpg-toolkit/core"
	"github.com/KirkDiggler/rpg-toolkit/mind/perception"
	"github.com/KirkDiggler/rpg-toolkit/tools/spatial"
	"github.com/stretchr/testify/require"
	"testing"
)

// Illustrative bytes and identities ONLY, not canonical storage/wire types.
type rd508Knowledge struct {
	State     string            `json:"state,omitempty"`
	Position  *spatial.Position `json:"position,omitempty"`
	Contents  []string          `json:"contents,omitempty"`
	DoorState DoorStateKind     `json:"door_state,omitempty"`
}
type rd508ProbeReach struct {
	e       *Encounter
	targets map[core.EntityID]spatial.Position
}

func (r rd508ProbeReach) Reaches(_ perception.Channel, observer, subject core.EntityID) bool {
	from, ok := r.e.canvas.GetEntityPosition(string(observer))
	if !ok {
		return false
	}
	if subject == "rd508:door" {
		for _, edge := range r.e.doorsByID["reference-tomb-heirloom/vault"].edges {
			if rd508Reaches(r.e, from, edge.From) || rd508Reaches(r.e, from, edge.To) {
				return true
			}
		}
		return false
	}
	to, ok := r.targets[subject]
	return ok && rd508Reaches(r.e, from, to)
}
func rd508Bytes(t *testing.T, k rd508Knowledge) []byte {
	t.Helper()
	raw, err := json.Marshal(k)
	require.NoError(t, err)
	return raw
}
func rd508Read(t *testing.T, p *perception.Perception, observer, subject core.EntityID) (rd508Knowledge, perception.Holding) {
	t.Helper()
	h, err := p.On(observer, subject)
	require.NoError(t, err)
	var k rd508Knowledge
	require.NoError(t, json.Unmarshal(h.Payload, &k))
	return k, h
}
func rd508ReloadKnowledge(t *testing.T, p *perception.Perception) *perception.Perception {
	t.Helper()
	raw, err := json.Marshal(p.ToData())
	require.NoError(t, err)
	var data perception.Data
	require.NoError(t, json.Unmarshal(raw, &data))
	out, err := perception.Load(data)
	require.NoError(t, err)
	require.Equal(t, p.ToData(), out.ToData())
	return out
}
func TestRD508KnowledgeAgainstTombSight(t *testing.T) {
	for _, shape := range []string{"subject", "place"} {
		t.Run(shape, func(t *testing.T) {
			e := rd508Tomb(t)
			p, err := perception.New()
			require.NoError(t, err)
			const content core.EntityID = "rd508:content"
			const doorID core.EntityID = "rd508:door"
			old := e.field.cellAt(e.field.props[e.field.propIndexOf("heirloom")].At)
			// The probe tests either-side endpoint reach for THIS bounded door.
			// One fixed anchor failed to report the door from both sides of a wall.
			// This does not settle arbitrary edges, rectangles, or partial footprints.
			support := e.doorsByID["reference-tomb-heirloom/vault"].edges[0].From
			gone := false
			var at uint64
			update := func() {
				at++
				targets := map[core.EntityID]spatial.Position{content: old, doorID: support}
				state := e.doorsByID["reference-tomb-heirloom/vault"].state.Kind()
				presences := []perception.Presence{{ID: doorID, Payload: rd508Bytes(t, rd508Knowledge{DoorState: state})}}
				if shape == "place" {
					contents := []string{}
					if !gone {
						contents = append(contents, "heirloom")
					}
					presences = append(presences, perception.Presence{ID: content, Payload: rd508Bytes(t, rd508Knowledge{Contents: contents})})
				} else if !gone {
					presences = append(presences, perception.Presence{ID: content, Payload: rd508Bytes(t, rd508Knowledge{State: "known", Position: &old})})
				}
				// A complete percept, including the actual placed roster. No second sight
				// pass for props that would fade the creature subjects by omission.
				for id := range e.members {
					cell, ok := e.canvas.GetEntityPosition(string(id))
					if ok {
						targets[id] = cell
						presences = append(presences, perception.Presence{ID: id, Payload: rd508Bytes(t, rd508Knowledge{State: "known", Position: &cell})})
					}
				}
				_, err = p.Observe(perception.Pass{At: at, Channel: perception.Sight, Observers: []core.EntityID{"alice", "bob"}, Presences: presences, Reach: rd508ProbeReach{e: e, targets: targets}})
				require.NoError(t, err)
				if shape == "subject" && gone {
					for _, observer := range []core.EntityID{"alice", "bob"} {
						remembered, h := rd508Read(t, p, observer, content)
						from, _ := e.canvas.GetEntityPosition(string(observer))
						if !h.CurrentOn(perception.Sight) && remembered.Position != nil && rd508Reaches(e, from, *remembered.Position) {
							_, err = p.Report(perception.ReportInput{At: at, Channel: perception.Sight, Observer: observer, Reports: []perception.Presence{{ID: content, Payload: rd508Bytes(t, rd508Knowledge{State: "unknown"})}}})
							require.NoError(t, err)
						}
					}
				}
			}
			update()
			initial, _ := rd508Read(t, p, "alice", content)
			rd508Walk(t, e, spatial.Position{X: 27, Y: 3})
			update()
			a, h := rd508Read(t, p, "alice", content)
			require.Equal(t, initial, a)
			require.False(t, h.CurrentOn(perception.Sight))
			_, err = e.Hold(&HoldInput{Member: "bob", Target: "heirloom"})
			require.NoError(t, err)
			gone = true
			update()
			a, _ = rd508Read(t, p, "alice", content)
			require.Equal(t, initial, a)
			b, h := rd508Read(t, p, "bob", content)
			if shape == "subject" {
				require.Equal(t, "unknown", b.State)
				require.Nil(t, b.Position)
				require.False(t, h.CurrentOn(perception.Sight))
			} else {
				require.Empty(t, b.Contents)
			}
			_, err = e.CloseDoor(&CloseDoorInput{Actor: "bob", Door: "reference-tomb-heirloom/vault"})
			require.NoError(t, err)
			update()
			rememberedDoor, _ := rd508Read(t, p, "alice", doorID)
			require.Equal(t, DoorOpen, rememberedDoor.DoorState)
			observedDoor, _ := rd508Read(t, p, "bob", doorID)
			require.Equal(t, DoorClosed, observedDoor.DoorState)
			e = rd508RoundTrip(t, e)
			p = rd508ReloadKnowledge(t, p)
			update()
			a, _ = rd508Read(t, p, "alice", content)
			require.Equal(t, initial, a)
			rd508Walk(t, e, spatial.Position{X: 27, Y: 5})
			update()
			observedDoor, _ = rd508Read(t, p, "alice", doorID)
			require.Equal(t, DoorClosed, observedDoor.DoorState)
			a, _ = rd508Read(t, p, "alice", content)
			require.Equal(t, initial, a, "seeing the door is not seeing the old prop position")
			_, err = e.OpenDoor(&OpenDoorInput{Actor: "alice", Door: "reference-tomb-heirloom/vault"})
			require.NoError(t, err)
			// With door open but an applicable runtime obstruction, no empty witness.
			require.NoError(t, e.AddSightArea(&SightAreaInput{ID: "probe-fog", SourceID: "probe", Center: old, RadiusFeet: 5}))
			update()
			a, _ = rd508Read(t, p, "alice", content)
			require.Equal(t, initial, a, "LOS alone must not bypass runtime obstruction")
			require.True(t, e.RemoveSightArea("probe"))
			update()
			a, h = rd508Read(t, p, "alice", content)
			if shape == "subject" {
				require.Equal(t, "unknown", a.State)
				require.Nil(t, a.Position)
				require.False(t, h.CurrentOn(perception.Sight))
			} else {
				require.Empty(t, a.Contents)
				require.True(t, h.CurrentOn(perception.Sight))
			}
			held, err := p.Held("alice")
			require.NoError(t, err)
			require.Len(t, held, 3, "one latest slot each for content, door, other character; no history")
			before := h.Payload
			update()
			_, h = rd508Read(t, p, "alice", content)
			require.Equal(t, before, h.Payload)
			e = rd508RoundTrip(t, e)
			p = rd508ReloadKnowledge(t, p)
			a, _ = rd508Read(t, p, "alice", content)
			require.Nil(t, a.Position)
			require.Empty(t, a.Contents)
			t.Log("real Tomb range/LOS/door/fog queries support independent memory and replacement with existing primitives; absence and door-unit composition are experimental")
		})
	}
}
