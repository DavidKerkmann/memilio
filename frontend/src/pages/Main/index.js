import React, {Component} from 'react';
import {Button, ButtonGroup, UncontrolledTooltip} from 'reactstrap';
import RtChart from '../../components/Graphs/RtChart';

import HeatMap from '../../common/heat-map';

import * as _ from 'lodash';
import * as dayjs from 'dayjs';
import 'dayjs/locale/de';

import './styles.scss';
import SimpleTimeline from '../../components/SimpleTimeline';

/**
 *  This component is the main page displayed. It shows the reproduction vales RT and RT relative
 *  on an choropleth county map as well as a line chart for individual counties.
 */

class MainPage extends Component {
  /** @type HeatMap */
  map = null;

  /** @type Map<number, Map<string, number>> */
  map_data_rt = null;

  /** @type Map<number, Map<string, number>> */
  map_data_rt_rel = null;

  /** @type Map<number, Map<string, number>> */
  map_data_incidence = null;

  /** @type Map<string, array> */
  chart_data = null;

  constructor(props) {
    super(props);
    this.state = {
      dataset: 'absolute',
      selected: {rs: '', bez: '', gen: ''},
    };
    this.update = this.update.bind(this);
  }

  async componentDidMount() {
    document.title = `SARS-CoV-2 Reproduktionszahlen`;
    // fetch rt data
    const data = await fetch('assets/rt.rel.districts.json').then((res) => res.json());

    const keys = Object.keys(data);

    const districts = _.uniq(data.DistrictID);
    const timestamps = _.uniq(data.Timestamp);
    const first_timestamp_idx = data.Rt.findIndex((d) => d !== 'NA');
    const first_timestamp = data.Timestamp[first_timestamp_idx];

    const num_items = data.Timestamp.length;

    this.map_data_rt = new Map();
    this.map_data_rt_rel = new Map();
    this.map_data_incidence = new Map();
    this.chart_data = new Map();

    for (let i = 0; i < num_items; i++) {
      const id = data.DistrictID[i];
      if (!this.chart_data.has(id)) {
        this.chart_data.set(id, []);
      }

      this.chart_data.get(id).push(
        keys.reduce((acc, c, idx) => {
          acc[c.toLowerCase()] = data[c][i];

          if (acc[c.toLowerCase()] === 'NA') {
            delete acc[c.toLowerCase()];
          }
          return acc;
        }, {})
      );
    }

    for (let i = first_timestamp_idx; i < timestamps.length; i++) {
      const x = districts.map((id) => this.chart_data.get(id).find((e) => e.timestamp === timestamps[i]));

      this.map_data_rt.set(
        timestamps[i],
        x.reduce((acc, c, idx) => {
          acc.set(c.districtid, c.rt);
          return acc;
        }, new Map())
      );

      this.map_data_rt_rel.set(
        timestamps[i],
        x.reduce((acc, c, idx) => {
          acc.set(c.districtid, c.rt_rel);
          return acc;
        }, new Map())
      );

      this.map_data_incidence.set(
        timestamps[i],
        x.reduce((acc, c, idx) => {
          acc.set(c.districtid, c.incidence_week);
          return acc;
        }, new Map())
      );
    }

    this.setState({
      selected: {rs: '00000', bez: 'Deutschland', gen: ''},
      timestamps: timestamps,
      timestampOffset: first_timestamp_idx,
      timestring: dayjs(timestamps[timestamps.length - 1])
        .locale('de')
        .format('DD MMMM YYYY'),
      //timestep: timestamps.findIndex((x) => x === first_timestamp),
      timestep: timestamps.length,
      start: timestamps.findIndex((x) => x === first_timestamp),
      end: timestamps.length,
    });

    this.map = new HeatMap('map', {showLegend: true});
    this.map.setLegendMinMax(0, 2);

    // wait a little to let amcharts finish render
    setTimeout((x) => {
      this.map.setValues(this.map_data_rt.get(timestamps[timestamps.length - 1]));

      // subscribe to events from map
      this.map.subscribe((event) => {
        switch (event) {
          case 'ready':
            break;
          case 'reset':
            this.setState({
              selected: {rs: '00000', bez: 'Deutschland', gen: ''},
            });
            break;
          default:
            this.setState({
              selected: event,
            });
        }
      });
    }, 2000);
  }

  /**
   * Update the currently displayed timestep.
   *
   * @param number timestep
   */
  update(timestep) {
    if (this.map) {
      timestep = this.state.timestampOffset + timestep;
      const timestamp = this.state.timestamps[timestep];
      this.setState({
        timestep,
        timestring: dayjs(timestamp).locale('de').format('DD MMMM YYYY'),
      });
      this.map.setValues(this.getData(timestamp));
    }
  }

  /**
   * Retrives the data from the currently selected dataset for given timestamp.
   *
   * @param number timestamp
   * @return Map<string, value>
   */
  getData(timestamp) {
    let data = null;
    switch (this.state.dataset) {
      case 'absolute':
        data = this.map_data_rt.get(timestamp);
        break;

      case 'relative':
        data = this.map_data_rt_rel.get(timestamp);
        break;

      case 'incidence':
        data = this.map_data_incidence.get(timestamp);
        break;

      default:
        break;
    }
    return data;
  }

  getChartData() {
    if (!this.chart_data) {
      return [];
    }
    let data = JSON.parse(JSON.stringify(this.chart_data.get(this.state.selected.rs)));

    switch (this.state.dataset) {
      case 'absolute':
        data = data.map((d) => {
          delete d['incidence_week'];
          return d;
        });
        break;
      case 'relative':
        data = data.map((d) => {
          delete d['incidence_week'];
          return d;
        });
        break;
      case 'incidence':
        data = data.map((d) => {
          delete d['rt_rel'];
          delete d['rt'];
          return d;
        });
        break;
      default:
        data = [];
    }

    console.log('final data', JSON.stringify(data));
    return data;
  }

  /**
   * Set selected dataset.
   *
   * @param number timestamp
   * @return Map<string, value>
   */
  selectDataset(dataset) {
    if (dataset === 'incidence') {
      // reset legend to auto set legend scale
      this.map.setLegendMinMax(0, 200);
    } else {
      // fix legend between 0 and 2
      this.map.setLegendMinMax(0, 2);
    }
    this.setState(
      {
        dataset: dataset,
      },
      () => {
        this.map.setValues(this.getData(this.state.timestamps[this.state.timestep]));
      }
    );
  }

  render() {
    return (
      <div className="main">
        <div className="left">
          <div className="timeline">
            <SimpleTimeline
              start={this.state.start}
              end={this.state.end}
              value={this.state.timestep}
              onChange={this.update}
            />
            <div className="timestring">{this.state.timestring}</div>
            <div className="options">
              <ButtonGroup>
                <Button
                  color="primary"
                  size="sm"
                  onClick={() => this.selectDataset('absolute')}
                  active={this.state.dataset === 'absolute'}
                  id="absolute"
                >
                  Rt Absolut
                </Button>
                <Button
                  color="primary"
                  size="sm"
                  onClick={() => this.selectDataset('relative')}
                  active={this.state.dataset === 'relative'}
                  id="relative"
                >
                  Rt Relativ
                </Button>
                <Button
                  color="primary"
                  size="sm"
                  onClick={() => this.selectDataset('incidence')}
                  active={this.state.dataset === 'incidence'}
                  id="incidence"
                >
                  7-Tage Inzidenz
                </Button>
                <UncontrolledTooltip placement="top" target="absolute">
                  Visualisiert die aktuelle Reproduktionszahl pro Landkreis.
                </UncontrolledTooltip>
                <UncontrolledTooltip placement="top" target="relative">
                  Visualisiert die aktuelle Reproduktionszahl pro Landkreis in Relation zur Reproduktionszahl
                  Deutschlands
                </UncontrolledTooltip>
                <UncontrolledTooltip placement="top" target="incidence">
                  7-Tage Inzidenz
                </UncontrolledTooltip>
              </ButtonGroup>
            </div>
          </div>
          <div className="map" id="map"></div>
        </div>
        <div className="right">
          <div className="graph">
            <div className="links">
              {this.state.selected.rs === '00000' || this.state.selected.rs.length === 0 ? (
                <></>
              ) : (
                <Button
                  color="link"
                  onClick={() => {
                    this.setState({
                      selected: {rs: '00000', bez: 'Deutschland', gen: ''},
                    });
                  }}
                >
                  Deutschland
                </Button>
              )}
            </div>
            <div className="district">
              {this.state.selected.bez} {this.state.selected.gen}
            </div>
            {this.state.dataset === 'incidence' ? (
              <RtChart
                id="incidence"
                series={[{key: 'incidence_week', label: '7-Tage Inzidenz'}]}
                data={this.getChartData()}
                district={this.state.selected.rs}
                dataset={this.state.dataset}
              />
            ) : (
              <RtChart
                id="rt"
                series={[
                  {key: 'rt', label: 'RT absolut'},
                  {key: 'rt_rel', label: 'RT relativ', isHidden: this.state.selected.rs === '00000'},
                ]}
                data={this.getChartData()}
                district={this.state.selected.rs}
                dataset={this.state.dataset}
              />
            )}
          </div>
        </div>
      </div>
    );
  }
}

export default MainPage;
