/*  This file is part of UnrealFusion, a sensor fusion plugin for VR in the Unreal Engine
    Copyright (C) 2017 Jake Fountain
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "UnrealFusion.h"
#include "Core.h"
namespace fusion {

	void Core::addNode(const NodeDescriptor & node, const NodeDescriptor & parent)
	{
		skeleton.addNode(node, parent);
	}

	void Core::finaliseSetup()
	{
	}

	//Adds a new measurement to the system
	void Core::addMeasurement(const Measurement::Ptr& m, const NodeDescriptor& node) {
		m->getSensor()->addNode(node);
		measurement_buffer.push_back(m);
	}

	//Adds a new measurement to the system
	void Core::addMeasurement(const Measurement::Ptr& m, const std::vector<NodeDescriptor>& nodes) {
		//Add nodes which the measurement might correspond to - actually gets stored in the sensor pointer
		for(auto& n : nodes){
			m->getSensor()->addNode(n);
		}
		measurement_buffer.push_back(m);
	}

	//Computes data added since last fuse() call. Should be called repeatedly	
	void Core::fuse() {
		//Add new data to calibration, with checking for usefulness

		correlator.addMeasurementGroup(measurement_buffer);
		correlator.identify();
		if(correlator.isStable()){
			calibrator.addMeasurementGroup(measurement_buffer);
			calibrator.calibrate();
			if(calibrator.isStable()){
				//skeleton.addMeasurementGroup(measurement_buffer);
				skeleton.fuse();
			}
		}
		

		measurement_buffer.clear();
	}

	CalibrationResult Core::getCalibrationResult(SystemDescriptor s1, SystemDescriptor s2) {
		return calibrator.getResultsFor(s1, s2);
	}

	//Called by owner of the Core object
	void Core::setMeasurementSensorInfo(Measurement::Ptr & m, SystemDescriptor system, SensorID id)
	{
		//If we haven't seen this sensor already, initialise
		if (utility::safeAccess(sensors, system).count(id) == 0) {
			utility::safeAccess(sensors, system)[id] = std::make_unique<Sensor>();
			utility::safeAccess(sensors, system)[id]->system = system;
			utility::safeAccess(sensors, system)[id]->id = id;
		}
		//Set pointer in measurement
		m->setSensor(sensors[system][id]);
	}

}
