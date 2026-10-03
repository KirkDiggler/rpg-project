package dungeonspec_test

import (
	"encoding/json"
	"github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e/encounter"
	"github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e/encounter/dungeonspec"
	"github.com/KirkDiggler/rpg-toolkit/tools/spatial"
	"github.com/stretchr/testify/require"
	"os"
	"testing"
)

// Investigation only: actual API-authored geometry, bounded two-player roster.
// Creature play and production character capabilities are not exercised.
func TestRD508GenerateTomb(t *testing.T) {
	raw, err := os.ReadFile(os.Getenv("RD508_TOMB"))
	require.NoError(t, err)
	compiled, err := dungeonspec.Load(raw)
	require.NoError(t, err)
	enc, err := encounter.NewEncounter(&encounter.SetupInput{
		Field: compiled.Field,
		Members: []encounter.MemberInput{
			{ID: "alice", Kind: encounter.KindPlayer, Position: spatial.Position{X: 28, Y: 2}, SpeedFeet: 30},
			{ID: "bob", Kind: encounter.KindPlayer, Position: spatial.Position{X: 29, Y: 4}, SpeedFeet: 30},
		},
		Endings: []encounter.EndingInput{{Key: "artifact-out", Trigger: encounter.TriggerExitedHolding{Item: "heirloom", Exit: "entrance"}}},
		Sight:   everyoneSeesTheWholeMap{}, Equipment: noHandsAreObserved{}, Standing: everyoneStanding{}, Initiative: orderAsGiven{},
		TurnDriver: passDriver{}, Striker: noAttacksExpected{}, Mover: quietMover{}, Announcer: quietAnnouncer{},
		CheckResolver: nothingIsEverFound{}, Witness: nobodyPerceivesAnything{},
	})
	require.NoError(t, err)
	var found bool
	for _, door := range enc.Doors() {
		if door.ID == "reference-tomb-heirloom/vault" {
			found = true
			require.Equal(t, encounter.DoorOpen, door.State.Kind())
			t.Logf("vault runtime ID=%s; initial state=%s", door.ID, door.State.Kind())
		}
	}
	require.True(t, found)
	data, err := json.MarshalIndent(enc.ToData(), "", "  ")
	require.NoError(t, err)
	require.NoError(t, os.WriteFile(os.Getenv("RD508_DATA"), data, 0600))
	t.Logf("compiled API Tomb: %d regions, %d legacy props, %d footprint props, %d monsters excluded from bounded roster", len(compiled.Field.Regions), len(compiled.Field.Props), len(compiled.Field.Placed), len(compiled.Monsters))
}
